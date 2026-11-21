"""Generate an independent FLY project from a supported board and template."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from urllib.parse import urlsplit

from vscode_project import write_vscode_scaffold


PROJECT_NAME = re.compile(r"[A-Za-z0-9][A-Za-z0-9_-]*")
RESERVED_NAME = re.compile(r"(?:CON|PRN|AUX|NUL|COM[0-9]|LPT[0-9])", re.IGNORECASE)
GIT_COMMIT = re.compile(r"[0-9a-f]{40}(?:[0-9a-f]{24})?")
SSH_ORIGIN = re.compile(r"[\w.-]+@([\w.-]+):([^\s]+)")
DEFCONFIG_TOKEN = re.compile(r"@([A-Za-z_][A-Za-z0-9_]*)@")


def nonempty_line(value):
    return isinstance(value, str) and bool(value.strip()) and not any(char in value for char in "\r\n\0")


BOARD_FIELD_TYPES = {
    "integer": lambda value: type(value) is int,
    "positive_integer": lambda value: type(value) is int and value > 0,
    "string": nonempty_line,
}


@dataclass(frozen=True)
class Template:
    id: str
    directory: Path
    label: str
    expected_log: str
    overlays: tuple[Path, ...]
    defconfig: str
    board_bindings: dict[str, tuple[str, str]]


def add_parser(subparsers, templates):
    parser = subparsers.add_parser("new", help="Generate an independent FLY project.")
    parser.add_argument("name", nargs="?", help="Project name.")
    parser.add_argument("--board", help="Board ID from the SDK board manifests.")
    parser.add_argument("--template", choices=templates, help="Application template.")
    parser.add_argument(
        "--output", type=Path,
        help="Destination directory (defaults to ../fly-project/<name> next to FLY).",
    )


def read_manifest(path):
    try:
        metadata = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        raise ValueError(f"Invalid JSON in {path}: {error}") from error
    if not isinstance(metadata, dict):
        raise ValueError(f"Manifest must be an object: {path}")
    return metadata


def manifest_strings(metadata, key, path):
    items = metadata.get(key, [])
    if not isinstance(items, list) or not all(map(nonempty_line, items)):
        raise ValueError(f"{key} must be a list of nonempty strings: {path}")
    return items


def relative_manifest_path(value, manifest):
    parts = value.split("/")
    if any(part in ("", ".", "..") for part in parts) or any(char in value for char in ("\\", ":", "\0")):
        raise ValueError(f"Expected a relative path within the template tree, got {value!r}: {manifest}")
    return Path(*parts)


def contained_path(path, root, manifest):
    resolved = path.resolve()
    try:
        resolved.relative_to(root.resolve())
    except ValueError as error:
        raise ValueError(f"Path {path} escapes {root}: {manifest}") from error
    return resolved


def load_overlays(metadata, root, manifest):
    overlays = []
    for name in manifest_strings(metadata, "overlays", manifest):
        overlay = contained_path(root / relative_manifest_path(name, manifest), root, manifest)
        if not overlay.is_dir():
            raise ValueError(f"Overlay {name!r} is missing or is not a directory: {manifest}")
        overlays.append(overlay)
    return tuple(overlays)


def load_template(manifest, apps_dir):
    directory = contained_path(manifest.parent, apps_dir, manifest)
    metadata = read_manifest(contained_path(manifest, directory, manifest))
    if not nonempty_line(metadata.get("label")):
        raise ValueError(f"Template manifest needs a nonempty label: {manifest}")
    expected_log = metadata.get("expected_log", "")
    if "expected_log" in metadata and not nonempty_line(expected_log):
        raise ValueError(f"expected_log must be a nonempty string: {manifest}")

    overlays = load_overlays(metadata, apps_dir, manifest)
    manifest_strings(metadata, "expected_config", manifest)
    for name in manifest_strings(metadata, "required_files", manifest):
        relative_manifest_path(name, manifest)

    defconfig_path = contained_path(directory / "defconfig", directory, manifest)
    if not defconfig_path.is_file():
        raise ValueError(f"Template is missing its defconfig: {manifest}")
    defconfig = defconfig_path.read_text(encoding="utf-8")
    raw_bindings = metadata.get("board_bindings", {})
    if not isinstance(raw_bindings, dict):
        raise ValueError(f"board_bindings must be an object: {manifest}")
    bindings = {}
    for token, binding in raw_bindings.items():
        field = binding.get("field") if isinstance(binding, dict) else None
        kind = binding.get("type") if isinstance(binding, dict) else None
        if not nonempty_line(field) or not isinstance(kind, str) or kind not in BOARD_FIELD_TYPES:
            raise ValueError(f"Invalid board binding for @{token}@: {manifest}")
        bindings[token] = (field, kind)
    tokens = set(DEFCONFIG_TOKEN.findall(defconfig))
    if tokens != bindings.keys():
        differing = ", ".join(f"@{token}@" for token in sorted(tokens ^ bindings.keys()))
        raise ValueError(f"Board bindings must match defconfig placeholders ({differing}): {manifest}")
    return Template(
        manifest.parent.name, directory, metadata["label"], expected_log,
        overlays, defconfig, bindings,
    )


def load_templates(templates_dir):
    apps_dir = templates_dir / "apps"
    templates = {}
    for manifest in sorted(apps_dir.glob("*/template.json")):
        name = manifest.parent.name
        if not PROJECT_NAME.fullmatch(name) or RESERVED_NAME.fullmatch(name):
            raise ValueError(f"Invalid template ID: {manifest}")
        templates[name] = load_template(manifest, apps_dir)
    if not templates:
        raise ValueError(f"No templates found in {apps_dir}")
    return templates


def render_defconfig(template, board):
    values = {}
    for token, (field, kind) in template.board_bindings.items():
        value = board.get(field)
        if not BOARD_FIELD_TYPES[kind](value):
            description = kind.replace("_", " ")
            raise ValueError(f"Board {board['id']!r} needs {description} field {field!r} for @{token}@")
        values[token] = str(value)
    return DEFCONFIG_TOKEN.sub(lambda match: values[match[1]], template.defconfig)


def load_boards(templates_dir, templates):
    boards = {}
    boards_dir = templates_dir / "boards"
    for manifest in sorted(boards_dir.glob("*/board.json")):
        board = read_manifest(contained_path(manifest, boards_dir, manifest))
        for key in ("id", "label", "chip", "core", "arch_config", "toolchain", "probe"):
            if not isinstance(board.get(key), str) or not board[key]:
                raise ValueError(f"Board manifest needs {key}: {manifest}")
        if board["id"] != manifest.parent.name:
            raise ValueError(f"Board ID must match its directory: {manifest}")
        if not re.fullmatch(r"CONFIG_ARCH_[A-Z0-9_]+", board["arch_config"]):
            raise ValueError(f"Invalid architecture symbol: {manifest}")
        supported = board.get("templates")
        if not isinstance(supported, list) or not supported or not all(
            isinstance(name, str) and name in templates for name in supported
        ):
            raise ValueError(f"Board manifest lists missing or unknown templates: {manifest}")
        for name in supported:
            render_defconfig(templates[name], board)
        board["overlays"] = load_overlays(board, boards_dir, manifest)
        boards[board["id"]] = board
    if not boards:
        raise ValueError(f"No boards found in {templates_dir / 'boards'}")
    return boards


def choose(title, options):
    if len(options) == 1:
        return next(iter(options))
    numbered = {str(index): key for index, key in enumerate(options, start=1)}
    print(title)
    for index, key in numbered.items():
        print(f"  {index}. {options[key]} ({key})")
    while True:
        selected = input("Enter a number or ID: ").strip()
        if selected in options:
            return selected
        if selected in numbered:
            return numbered[selected]
        print("Select one of the listed options.")


def select_project(boards, templates, args):
    if not sys.stdin.isatty() and not all((args.name, args.board, args.template)):
        raise ValueError("Non-interactive generation needs <name>, --board and --template.")
    name = args.name if args.name is not None else input("Project name: ").strip()
    if not PROJECT_NAME.fullmatch(name) or RESERVED_NAME.fullmatch(name):
        raise ValueError("Use a portable project name with letters, digits, '-' or '_', starting with a letter or digit.")

    candidates = {
        board_id: board for board_id, board in boards.items()
        if args.template is None or args.template in board["templates"]
    }
    if args.board is not None:
        if args.board not in boards:
            raise ValueError(f"Unknown board {args.board!r}. Available boards: {', '.join(boards)}")
        if args.board not in candidates:
            supported = ", ".join(boards[args.board]["templates"])
            raise ValueError(f"{args.board} supports these templates: {supported}")
        board = boards[args.board]
    else:
        if not candidates:
            raise ValueError(f"No boards support {args.template}.")
        chips = dict.fromkeys(board["chip"] for board in candidates.values())
        chip = choose("Select a chip", {chip: chip for chip in chips})
        board_id = choose(
            "Select a board",
            {
                board_id: f'{board["label"]} / {board["core"]}'
                for board_id, board in candidates.items() if board["chip"] == chip
            },
        )
        board = candidates[board_id]

    template = args.template or choose(
        "Select an application template",
        {key: templates[key].label for key in board["templates"]},
    )
    return name, board, templates[template]


def shareable_origin(origin):
    if not origin or any(character.isspace() for character in origin):
        return None
    ssh_origin = SSH_ORIGIN.fullmatch(origin)
    if ssh_origin:
        host, path = ssh_origin.groups()
        if host.lower() == "github.com":
            repository = path.lstrip("/")
            if re.fullmatch(r"[\w.-]+/[\w.-]+", repository):
                return f"https://github.com/{repository}"
            return None
        return origin
    try:
        url = urlsplit(origin)
        if not url.hostname or not url.path or url.query or url.fragment or url.password:
            return None
        if url.scheme in ("http", "https") and url.username is None:
            return origin
        if url.scheme in ("ssh", "git"):
            return origin
    except ValueError:
        pass
    return None


def git_output(sdk_dir, *args):
    result = subprocess.run(
        ["git", "-C", str(sdk_dir), *args], capture_output=True, text=True,
    )
    return result.stdout.strip() if result.returncode == 0 else ""


def sdk_lock(sdk_dir):
    # Read the configured URL before Git applies local URL rewrites.
    origin = shareable_origin(git_output(sdk_dir, "config", "--local", "--get", "remote.origin.url"))
    commit = git_output(sdk_dir, "rev-parse", "--verify", "HEAD")
    entry = git_output(sdk_dir, "cat-file", "-t", "HEAD:cmake/fly-configure.cmake")
    if not GIT_COMMIT.fullmatch(commit) or entry != "blob":
        commit = None
    return {"url": origin, "commit": commit}


def copy_template_layer(source, destination):
    def ignore_manifest(directory, _names):
        return ("template.json",) if Path(directory) == source else ()

    shutil.copytree(source, destination, dirs_exist_ok=True, ignore=ignore_manifest)


def generate(sdk_dir, destination, name, board, template):
    templates_dir = sdk_dir / "templates"
    selected = render_defconfig(template, board)
    lock = sdk_lock(sdk_dir)
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Reserve this name before copying. Concurrent generators or an existing
    # directory, file, or symlink cannot be overwritten.
    destination.mkdir()
    try:
        shutil.copytree(templates_dir / "scaffold", destination, dirs_exist_ok=True)
        board_directory = templates_dir / "boards" / board["id"]
        for layer in (*board["overlays"], board_directory):
            shutil.copytree(layer, destination / "board", dirs_exist_ok=True)
        for layer in (*template.overlays, template.directory):
            copy_template_layer(layer, destination)
        write_vscode_scaffold(destination)
        defconfig = destination / "defconfig"
        board_config = destination / "board/defconfig"
        board_defaults = board_config.read_text(encoding="utf-8").rstrip() + "\n" if board_config.is_file() else ""
        defconfig.write_text(f'{board["arch_config"]}=y\n{board_defaults}{selected}', encoding="utf-8")

        cmake_file = destination / "CMakeLists.txt"
        cmake_file.write_text(
            cmake_file.read_text(encoding="utf-8").replace("@PROJECT_NAME@", name),
            encoding="utf-8",
        )

        readme = destination / "README.md"
        content = readme.read_text(encoding="utf-8")
        startup_log_note = ""
        if template.expected_log:
            startup_log_note = f"The program's\nstartup output is `{template.expected_log}`. "
        fields = {
            "@PROJECT_NAME@": name,
            "@BOARD_LABEL@": board["label"],
            "@BOARD_CORE@": board["core"],
            "@TOOLCHAIN@": board["toolchain"],
            "@PROBE@": board["probe"],
            "@TEMPLATE_ID@": template.id,
            "@STARTUP_LOG_NOTE@": startup_log_note,
        }
        for token, value in fields.items():
            content = content.replace(token, value)
        readme.write_text(content, encoding="utf-8")

        (destination / "fly.lock").write_text(json.dumps(lock, indent=2) + "\n", encoding="utf-8")
        if os.name == "nt":
            (destination / ".fly-root").write_text(f"{sdk_dir}\n", encoding="utf-8")
        else:
            (destination / "build.sh").chmod(0o755)
            (destination / "fly").symlink_to(os.path.relpath(sdk_dir, destination), target_is_directory=True)
        subprocess.run(["git", "init", "--quiet"], cwd=destination, check=True)
        # The Git index keeps the launcher executable on filesystems without mode bits.
        subprocess.run(["git", "add", "--chmod=+x", "--", "build.sh"], cwd=destination, check=True)
    except BaseException:
        shutil.rmtree(destination)
        raise
    return lock


def run(sdk_dir, templates, args):
    sdk_dir = Path(sdk_dir).resolve()
    try:
        boards = load_boards(sdk_dir / "templates", templates)
        name, board, template = select_project(boards, templates, args)
        output = args.output.expanduser() if args.output is not None else sdk_dir.parent / "fly-project" / name
        destination = output.parent.resolve() / output.name
        lock = generate(sdk_dir, destination, name, board, template)
    except FileExistsError as exc:
        print(f"Cannot create project because a path already exists: {exc.filename}", file=sys.stderr)
        return 1
    except (ValueError, OSError, EOFError, subprocess.CalledProcessError) as exc:
        print(f"Cannot create project: {exc}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nProject generation cancelled.", file=sys.stderr)
        return 130

    print(f"Created {destination}")
    print(f'{board["label"]} / {board["core"]} | {template.label}')
    if lock["commit"] is None:
        print(
            "FLY HEAD does not support application-root CMake. "
            "Set fly.lock to a published FLY commit before sharing this project.",
            file=sys.stderr,
        )
    elif git_output(sdk_dir, "status", "--porcelain"):
        print(
            "fly.lock records FLY HEAD, while generated files can include local changes. "
            "Before sharing, publish a compatible FLY commit and update fly.lock.",
            file=sys.stderr,
        )
    if lock["url"] is None:
        print("Set a shareable FLY repository URL in fly.lock before sharing this project.", file=sys.stderr)
    print("To build, run ./build.sh (or build.bat on Windows) from the new project.")
    return 0


def main():
    sdk_dir = Path(__file__).resolve().parents[2]
    try:
        templates = load_templates(sdk_dir / "templates")
    except (ValueError, OSError) as error:
        print(f"Cannot load templates: {error}", file=sys.stderr)
        return 1
    parser = argparse.ArgumentParser(prog="fly", description=__doc__)
    add_parser(parser.add_subparsers(dest="command", required=True), templates)
    return run(sdk_dir, templates, parser.parse_args())


if __name__ == "__main__":
    sys.exit(main())
