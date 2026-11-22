import subprocess
import sys
import tempfile
import unittest
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "python"))

from git_source_cache import GitSourceCache


class GitSourceCacheTests(unittest.TestCase):
    def setUp(self):
        workspace = tempfile.TemporaryDirectory(prefix="fly shared git ")
        self.addCleanup(workspace.cleanup)
        self.root = Path(workspace.name)
        self.remote = self.root / "vendor repo"
        self.git("init", "-q", "-b", "main", self.remote)
        self.git("-C", self.remote, "config", "uploadpack.allowFilter", "true")
        self.git("-C", self.remote, "config", "uploadpack.allowAnySHA1InWant", "true")
        self.vendor = self.remote / "chip"
        self.vendor.mkdir()
        other = self.remote / "other"
        other.mkdir()
        (other / "unused.c").write_text("unused\n", encoding="utf-8")
        self.first = self.commit("old\n")
        self.git("-C", self.remote, "-c", "user.name=FLY", "-c", "user.email=fly@example.invalid",
                 "-c", "tag.gpgSign=false", "tag", "-a", "v1", "-m", "Version 1")
        self.cache = GitSourceCache(self.root / "sdk cache")

    def git(self, *args):
        return subprocess.check_output(["git", *map(str, args)], text=True).strip()

    def commit(self, text):
        (self.vendor / "part.c").write_text(text, encoding="utf-8")
        self.git("-C", self.remote, "add", ".")
        self.git("-C", self.remote, "-c", "user.name=FLY", "-c", "user.email=fly@example.invalid",
                 "-c", "commit.gpgsign=false", "commit", "-qm", text.strip())
        return self.git("-C", self.remote, "rev-parse", "HEAD")

    def fetch(self, app, ref=None, **options):
        return self.cache.fetch("vendor", self.remote.as_uri(), ref or self.first,
                                result_file=self.root / app / "source.json", **options)

    def patch(self, path, replacement):
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("diff --git a/chip/part.c b/chip/part.c\n"
                        "--- a/chip/part.c\n+++ b/chip/part.c\n"
                        f"@@ -1 +1 @@\n-old\n+{replacement}\n", encoding="utf-8")

    def test_parallel_apps_share_clean_commit_and_keep_patch_variants_isolated(self):
        downloads = []

        def mirror(url):
            downloads.append(url)
            return url

        with ThreadPoolExecutor(max_workers=2) as pool:
            first, second = list(pool.map(lambda app: self.fetch(app, mirror=mirror), ("app one", "app two")))
        self.assertEqual(first, second)
        self.assertEqual(len(downloads), 1)
        original = Path(first["source_dir"])
        self.assertEqual(original.name, f"vendor-{self.first}")

        patch_root = self.root / "patches"
        patch = patch_root / self.first / "01-platform.patch"
        self.patch(patch, "custom one")
        self.remote.rename(self.root / "disconnected")
        custom = self.fetch("app one", patch_dirs=[patch_root])
        other = Path(custom["source_dir"])
        self.assertEqual((other / "chip/part.c").read_text(), "custom one\n")
        self.assertEqual(self.git("-C", other, "rev-parse", "HEAD"), self.first)
        timestamp = (self.root / "app one/source.json").stat().st_mtime_ns
        self.assertEqual(self.fetch("app one", patch_dirs=[patch_root]), custom)
        self.assertEqual((self.root / "app one/source.json").stat().st_mtime_ns, timestamp)

        self.patch(patch, "custom two")
        latest = Path(self.fetch("app one", patch_dirs=[patch_root])["source_dir"])
        self.assertNotEqual(latest, other)
        self.assertEqual((latest / "chip/part.c").read_text(), "custom two\n")
        self.assertEqual((original / "chip/part.c").read_text(), "old\n")
        self.assertEqual((other / "chip/part.c").read_text(), "custom one\n")
        result_file = self.root / "app one/source.json"
        before = result_file.read_bytes()
        directories = sorted(self.cache.root.glob("vendor-*"))
        patch.write_text("invalid patch\n", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "does not apply to"):
            self.fetch("app one", patch_dirs=[patch_root])
        self.assertEqual(result_file.read_bytes(), before)
        self.assertEqual(sorted(self.cache.root.glob("vendor-*")), directories)

    def test_branch_refresh_is_application_local_and_annotated_tag_pins_commit(self):
        initial = self.fetch("app one", "main")
        self.assertEqual(self.fetch("app two", "main"), initial)
        second = self.commit("next\n")
        refreshed = self.fetch("app two", "main", refresh=True)
        self.assertEqual(refreshed["commit"], second)
        self.assertEqual(self.fetch("app one", "main"), initial)
        self.assertEqual(self.fetch("new app", "main"), refreshed)
        self.assertEqual(self.fetch("tagged app", "v1")["commit"], self.first)
        patch_root = self.root / "patches"
        self.patch(patch_root / self.first / "01-old.patch", "old patch")
        self.assertEqual(self.fetch("app two", second, patch_dirs=[patch_root])["patch_id"], None)

    def test_sparse_blobs_are_shared_with_a_new_patch_variant_offline(self):
        first = self.fetch("app one", sparse_paths=["chip"])
        source = Path(first["source_dir"])
        self.assertTrue((source / "chip/part.c").is_file())
        self.assertFalse((source / "other/unused.c").exists())
        later = self.commit("new\n")
        distinct = self.fetch("different version", later, sparse_paths=["chip"])
        self.assertEqual((Path(distinct["source_dir"]) / "chip/part.c").read_text(), "new\n")
        self.remote.rename(self.root / "disconnected")
        patch_root = self.root / "patches"
        self.patch(patch_root / self.first / "01-platform.patch", "offline custom")
        second = self.fetch("app two", sparse_paths=["chip"], patch_dirs=[patch_root])
        self.assertEqual((Path(second["source_dir"]) / "chip/part.c").read_text(), "offline custom\n")
        moved = self.root / "moved cache"
        self.cache.root.rename(moved)
        self.cache = GitSourceCache(moved)
        copied = Path(self.fetch("app two", sparse_paths=["chip"], patch_dirs=[patch_root])["source_dir"])
        self.assertEqual(self.git("-C", copied, "rev-parse", "HEAD^{commit}"), self.first)

    def test_sparse_checkout_fetches_blobs_through_the_mirror(self):
        absent = (self.root / "unreachable origin").as_uri()
        result = self.cache.fetch("vendor", absent, self.first, sparse_paths=["chip"],
                                  mirror=lambda _: self.remote.as_uri(), result_file=self.root / "mirror.json")
        source = Path(result["source_dir"])
        self.assertEqual((source / "chip/part.c").read_text(), "old\n")
        self.assertEqual(self.git("-C", source, "config", "--get", "remote.origin.url"), absent)

    def test_two_repository_urls_can_publish_the_same_snapshot(self):
        def fetch(repository):
            return self.cache.fetch("vendor", repository, self.first,
                                    result_file=self.root / f"{repository.startswith('file:')}.json")

        with ThreadPoolExecutor(max_workers=2) as pool:
            a, b = list(pool.map(fetch, (self.remote.as_uri(), str(self.remote))))
        self.assertEqual(a["source_dir"], b["source_dir"])

    def test_deep_request_upgrades_a_cached_shallow_repository(self):
        new = self.commit("following\n")
        shallow = Path(self.fetch("app one", new)["source_dir"])
        self.assertEqual(self.git("-C", shallow, "rev-list", "--count", "HEAD"), "1")
        full = Path(self.fetch("app two", new, deep=True)["source_dir"])
        self.assertNotEqual(full, shallow)
        self.assertEqual(self.git("-C", full, "rev-list", "--count", "HEAD"), "2")
        self.assertEqual(self.git("-C", full, "rev-parse", "v1^{commit}"), self.first)
        self.assertEqual(self.git("-C", shallow, "rev-list", "--count", "HEAD"), "1")


if __name__ == "__main__":
    unittest.main()
