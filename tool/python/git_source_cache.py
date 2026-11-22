"""Commit-addressed Git sources shared by applications using the same cache."""

import json
import os
from hashlib import sha256
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
from tempfile import TemporaryDirectory

from action_protocol import read_json, write_json
from file_lock import file_lock


COMMIT = re.compile(r"[0-9a-f]{40}")
NAME = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]*")


def _git(*args, data=None, optional=False, allow_lazy=False):
    environment = {**os.environ, "GIT_TERMINAL_PROMPT": "0"}
    if not allow_lazy:
        environment["GIT_NO_LAZY_FETCH"] = "1"
    result = subprocess.run(
        ["git", *(str(arg) for arg in args)], input=data, capture_output=True,
        env=environment, timeout=300,
    )
    if result.returncode:
        if optional:
            return None
        raise ValueError(result.stderr.decode(errors="replace").strip() or "Git command failed")
    return result.stdout.decode(errors="replace").strip()


def _digest(value):
    return sha256(json.dumps(value, sort_keys=True, separators=(",", ":")).encode()).hexdigest()


def _metadata(path):
    try:
        value = read_json(path)
    except (OSError, ValueError):
        return {}
    return value if isinstance(value, dict) else {}


def write_if_changed(path, content):
    if path.is_file() and path.read_text(encoding="utf-8") == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    with TemporaryDirectory(prefix=".fly-result-", dir=path.parent) as temp:
        stage = Path(temp) / "result"
        stage.write_text(content, encoding="utf-8")
        stage.replace(path)


def _patches(directories, commit):
    for root in directories:
        for directory in (root, root / commit):
            for path in sorted(directory.glob("*.patch")):
                if path.is_file():
                    yield path, path.read_bytes()


def _has_full_history(git_dir, commit):
    shallow = git_dir / "shallow"
    if not shallow.is_file():
        return True
    boundaries = set(shallow.read_text(encoding="ascii").splitlines())
    return boundaries.isdisjoint(_git("--git-dir", git_dir, "rev-list", commit).splitlines())


class GitSourceCache:
    def __init__(self, root):
        self.root = Path(root).resolve()

    def fetch(self, name, repository, revision, *, result_file, patch_dirs=(), sparse_paths=(),
              deep=False, refresh=False, mirror=None):
        if not NAME.fullmatch(name):
            raise ValueError(f"Invalid Git source name {name!r}")
        if COMMIT.fullmatch(revision.lower()):
            revision = revision.lower()
        if not COMMIT.fullmatch(revision) and _git("check-ref-format", "--allow-onelevel", revision,
                                                   optional=True) is None:
            raise ValueError(f"Invalid Git revision {revision!r}")
        paths = sorted(set(sparse_paths))
        for path in paths:
            location = PurePosixPath(path)
            if path != location.as_posix() or location.is_absolute() or any(
                    not NAME.fullmatch(part) for part in location.parts) or not location.parts:
                raise ValueError(f"Invalid Git sparse directory {path!r}")
        repository_id = sha256(repository.encode()).hexdigest()
        git_dir = self.root / ".repos" / f"{repository_id}.git"
        with file_lock(self.root / ".locks" / f"{repository_id}.lock"):
            if not git_dir.exists():
                git_dir.parent.mkdir(parents=True, exist_ok=True)
                with TemporaryDirectory(prefix=".repo-", dir=git_dir.parent) as work:
                    stage = Path(work) / "repo.git"
                    _git("init", "--bare", "--quiet", stage)
                    stage.rename(git_dir)
            commit = self._resolve(git_dir, repository, revision, result_file,
                                   deep=deep, sparse=bool(paths), refresh=refresh, mirror=mirror)
            patches = list(_patches(patch_dirs, commit))
            checksums = [sha256(contents).hexdigest() for _, contents in patches]
            patch_id = _digest(checksums) if patches else None
            recipe = {"commit": commit, "patch_id": patch_id, "sparse_paths": paths}
            if deep:
                recipe["deep"] = True
            variant = f"-{_digest(recipe)[:16]}" if paths or patches or deep else ""
            source = self.root / f"{name}-{commit}{variant}"
            # Distinct repository URLs can resolve to the same content.
            with file_lock(self.root / ".locks" / f"source-{_digest(source.name.casefold())}.lock"):
                if source.exists():
                    if _metadata(source / ".git/fly-cache.json") != recipe:
                        raise ValueError(f"Unrecognised Git cache entry {source}; remove it and rebuild")
                else:
                    self._checkout(git_dir, source, repository, recipe, patches, mirror)
            value = {**recipe, "repository": repository, "revision": revision, "source_dir": source.as_posix()}
            write_if_changed(result_file, json.dumps(value, ensure_ascii=False, indent=2) + "\n")
            return value

    def _resolve(self, git_dir, repository, revision, result_file, *, deep, sparse, refresh, mirror):
        args = ("--git-dir", git_dir)
        alias = f"refs/fly/revisions/{_digest(revision)}"
        exact = bool(COMMIT.fullmatch(revision))
        previous = _metadata(result_file)
        selected = previous.get("commit") if (previous.get("repository") == repository
                                                and previous.get("revision") == revision) else None
        if not isinstance(selected, str) or not COMMIT.fullmatch(selected):
            selected = None
        known_commit = revision if exact else selected
        candidate = f"refs/fly/commits/{known_commit}" if known_commit else alias
        commit = _git(*args, "rev-parse", "--verify", "--end-of-options", f"{candidate}^{{commit}}", optional=True)
        if not commit and not exact and selected:
            commit = _git(*args, "rev-parse", "--verify", f"{alias}^{{commit}}", optional=True)
        use_cached = bool(commit and (exact or not refresh))
        tags_ready = git_dir / "fly-tags-ready"
        if use_cached and (not deep or tags_ready.is_file() and _has_full_history(git_dir, commit)):
            return commit

        options = ["--quiet", "--update-shallow"]
        if deep:
            options.extend(("--tags", "--force"))
            if (git_dir / "shallow").is_file():
                options.append("--unshallow")
        else:
            options.extend(("--depth=1", "--no-tags"))
        if sparse:
            options.append("--filter=blob:none")
        repos = dict.fromkeys((mirror(repository) if mirror else repository, repository))
        for remote in repos:
            try:
                _git(*args, "fetch", *options, "--", remote, commit if use_cached else revision)
                break
            except ValueError:
                if remote == repository:
                    raise
        commit = _git(*args, "rev-parse", "--verify", "FETCH_HEAD^{commit}")
        if exact and commit != revision:
            raise ValueError(f"Requested commit {revision}, resolved {commit}")
        if deep and not _has_full_history(git_dir, commit):
            raise ValueError(f"Full Git history is unavailable for {commit}")
        _git(*args, "update-ref", f"refs/fly/commits/{commit}", commit)
        if deep:
            tags_ready.touch()
        if not exact:
            _git(*args, "update-ref", alias, commit)
        return commit

    def _checkout(self, git_dir, destination, repository, recipe, patches, mirror):
        with TemporaryDirectory(prefix=".checkout-", dir=self.root) as work:
            tree = Path(work) / "source"
            _git("init", "--quiet", tree)
            args = ("-C", tree)
            _git(*args, "config", "core.autocrlf", "false")
            _git(*args, "remote", "add", "origin", repository)
            alternates = tree / ".git/objects/info/alternates"
            alternates.write_text(f"{(git_dir / 'objects').as_posix()}\n", encoding="utf-8")
            shallow = git_dir / "shallow"
            if shallow.is_file():
                shutil.copyfile(shallow, tree / ".git/shallow")
            # A sparse fetch leaves blobs as promised objects. Only the paths
            # selected for this application are fetched during checkout.
            packs = git_dir / "objects/pack"
            partial = bool(recipe["sparse_paths"]) or any(packs.glob("*.promisor"))
            if partial:
                _git(*args, "config", "remote.origin.promisor", "true")
                _git(*args, "config", "remote.origin.partialclonefilter", "blob:none")
            if recipe["sparse_paths"]:
                _git(*args, "sparse-checkout", "set", "--cone", "--", *recipe["sparse_paths"])
            remotes = dict.fromkeys((mirror(repository) if partial and mirror else repository, repository))
            for remote in remotes:
                _git(*args, "remote", "set-url", "origin", remote)
                try:
                    _git(*args, "checkout", "--quiet", "--force", "--detach", recipe["commit"], allow_lazy=partial)
                    break
                except ValueError:
                    if remote == repository:
                        raise
            _git(*args, "remote", "set-url", "origin", repository)
            if recipe.get("deep"):
                tags = _git("--git-dir", git_dir, "for-each-ref", "--format=update %(refname) %(objectname)", "refs/tags/")
                if tags:
                    _git(*args, "update-ref", "--stdin", data=(tags + "\n").encode())
            # Retain newly fetched blobs in the repository pool so other
            # sparse or patched checkouts can materialize without another fetch.
            for pattern in ("*.promisor", "*.pack", "*.idx"):
                for file in (tree / ".git/objects/pack").glob(pattern):
                    target = packs / file.name
                    if not target.exists():
                        file.rename(target)
            for path, contents in patches:
                try:
                    _git(*args, "apply", "--whitespace=nowarn", "-", data=contents)
                except ValueError as error:
                    raise ValueError(f"Patch {path} does not apply to {recipe['commit']}: {error}") from error
            write_json(tree / ".git/fly-cache.json", recipe)
            objects = Path(os.path.relpath(packs.parent, destination / ".git/objects"))
            alternates.write_text(f"{objects.as_posix()}\n", encoding="utf-8")
            tree.rename(destination)
