"""Contract and round-trip tests for the common firmware artifact."""

import json
from pathlib import Path
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import patch
from zipfile import ZipFile
import zlib

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "python"))

import firmware_payload as payload


class PayloadTests(unittest.TestCase):
    def setUp(self):
        scratch = TemporaryDirectory(prefix="firmware payload ")
        self.addCleanup(scratch.cleanup)
        self.project = Path(scratch.name)
        self.raw = self.project / "linked image.bin"
        self.raw.write_bytes(b"FIRMWARE\0")

    def package(self, *parts, **changes):
        options = {
            "target": "default", "build_type": "Release", "board_id": "board/model", "chip": "CHIP",
            "core_id": "core0", "firmware_name": "test", "firmware_parts": parts or (self.part(),),
        }
        options.update(changes)
        return payload.create_payload(self.project, **options)

    def part(self, name="APP", address="0x1000", capacity=160, path=None):
        return f"{name}:0.1.0:{address}:{capacity}:{path or self.raw}"

    def test_archive_round_trip_with_two_external_partitions(self):
        created = self.package(self.part(), self.part("BOOT", "0x2000"))
        manifest_path = Path(created["manifest_path"])
        self.assertEqual(payload.find_artifact(self.project, "default", "Release"), manifest_path)
        manifest = payload.load_artifact(manifest_path, artifact_id=created["artifact_id"])
        with ZipFile(manifest_path.parent.with_suffix(".zip")) as archive:
            extracted = self.project / "published archive"
            archive.extractall(extracted)
        self.raw.unlink()
        self.assertEqual(payload.load_artifact(extracted / "manifest.json"), manifest)
        self.assertEqual([part["name"] for part in manifest["partitions"]], ["APP", "BOOT"])
        for part in manifest["partitions"]:
            self.assertEqual(part["origin"], "external")
            self.assertIsNone(part["symbols"])
            self.assertEqual(part["bin"]["size"], 160)
            trailer = part["trailer"]
            final = payload.manifest_file(manifest_path, part["bin"]).read_bytes()
            position = trailer["offset"] + 16
            end = trailer["offset"] + trailer["size"]
            self.assertEqual(zlib.crc32(final[:position] + b"\xff" * 4 + final[position + 4:end]),
                             int(trailer["crc32"], 16))

    def test_existing_artifact_survives_invalid_size_overlap_and_failed_publication(self):
        initial = self.package()
        expected = Path(initial["manifest_path"])
        with self.assertRaises(payload.ArtifactError) as over:
            self.package(self.part(capacity=159))
        self.assertEqual(over.exception.code, "CAPACITY_EXCEEDED")
        with self.assertRaisesRegex(payload.ArtifactError, "overlap"):
            self.package(self.part(), self.part("BOOT", "0x1001"))

        self.raw.write_bytes(b"CHANGED\x00")
        with patch.object(payload, "_atomic_json", side_effect=OSError("publish interrupted")):
            with self.assertRaisesRegex(OSError, "publish interrupted"):
                self.package()
        self.assertEqual(payload.find_artifact(self.project, "default", "Release"), expected)
        self.assertEqual([path for path in expected.parent.parent.glob("*.zip")],
                         [expected.parent.with_suffix(".zip")])
        payload.load_artifact(expected)

    def test_cache_hits_then_detects_missing_file_and_corrupt_zip(self):
        first = self.package()
        self.assertEqual(self.package(), first)
        old = Path(first["manifest_path"])
        (old.parent / "APP.bin").unlink()
        repaired = self.package()
        self.assertNotEqual(first, repaired)
        current = Path(repaired["manifest_path"])
        archive = current.parent.with_suffix(".zip")
        archive.write_bytes(b"bad zip")
        self.assertNotEqual(self.package(), repaired)
        self.assertEqual(payload.load_artifact(current)["artifact_id"], repaired["artifact_id"])

    def test_consumer_rejects_invalid_references_and_schema(self):
        result = self.package()
        path = Path(result["manifest_path"])
        manifest = payload.load_artifact(path)
        with self.assertRaises(payload.ArtifactError) as wrong:
            payload.load_artifact(path, core_id="core1")
        self.assertEqual(wrong.exception.code, "ARTIFACT_MISMATCH")
        manifest["partitions"][0]["bin"]["path"] = "../APP.bin"
        path.write_text(json.dumps(manifest), encoding="utf-8")
        with self.assertRaisesRegex(payload.ArtifactError, "Unsafe archive path"):
            payload.load_artifact(path)
        path.write_text('{"schema_version":1,"schema_version":1}', encoding="utf-8")
        with self.assertRaisesRegex(payload.ArtifactError, "Repeated JSON key"):
            payload.load_artifact(path)

    def test_declared_board_flash_range_is_enforced_on_both_ends(self):
        regions = [{"kind": "flash", "address": 0x1000, "size": 160}]
        original = self.package(memories=regions)
        manifest = payload.load_artifact(original["manifest_path"], memories=regions)
        self.assertEqual(manifest["partitions"][0]["capacity"], 160)
        with self.assertRaisesRegex(payload.ArtifactError, "exceeds board flash"):
            self.package(self.part(address="0x2000"), memories=regions)
        other = self.package(self.part(address="0x2000"))
        with self.assertRaisesRegex(payload.ArtifactError, "exceeds board flash"):
            payload.load_artifact(other["manifest_path"], memories=regions)

    def test_target_and_type_are_distinct_and_current_is_pinned(self):
        release = self.package()
        debug = self.package(build_type="Debug")
        boot = self.package(target="recovery")
        for result, target, build_type in ((release, "default", "Release"), (debug, "default", "Debug"),
                                           (boot, "recovery", "Release")):
            self.assertEqual(payload.find_artifact(self.project, target, build_type),
                             Path(result["manifest_path"]))
        self.assertEqual(self.package(), release)

    def test_concurrent_publication_reuses_one_complete_pair(self):
        command = [
            sys.executable, str(Path(payload.__file__)), "--project-root", str(self.project),
            "--target", "default", "--build-type", "Release", "--board-id", "board/model",
            "--chip", "CHIP", "--core-id", "core0", "--firmware-name", "test", "--firmware-parts", self.part(),
        ]
        with subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True) as a:
            with subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True) as b:
                first, first_error = a.communicate(timeout=30)
                second, second_error = b.communicate(timeout=30)
                self.assertEqual(a.returncode, 0, first_error)
                self.assertEqual(b.returncode, 0, second_error)
        self.assertEqual(json.loads(first), json.loads(second))
        current = payload.find_artifact(self.project, "default", "Release")
        self.assertEqual(len(list(current.parent.parent.glob("*.zip"))), 1)

    @unittest.skipUnless(all(shutil.which(tool) for tool in ("arm-none-eabi-gcc", "arm-none-eabi-objcopy")),
                         "ARM GNU toolchain unavailable")
    def test_real_elf_pairs_raw_image_on_both_ends_and_uses_flash_lma(self):
        objcopy = shutil.which("arm-none-eabi-objcopy")
        source = self.project / "firmware.c"
        source.write_text("volatile unsigned initialized = 123;\n"
                          "void entry(void) {for (;;) { initialized++; }}\n", encoding="utf-8")
        linker = self.project / "firmware.ld"
        linker.write_text("ENTRY(entry)\n"
                          "SECTIONS { .text 0x1000 : { *(.text*) }\n"
                          " .data 0x20000000 : AT(0x1100) { *(.data*) } }\n", encoding="utf-8")
        elf = self.project / "firmware.elf"
        debug = self.project / "firmware.debug"
        commands = [
            ["arm-none-eabi-gcc", "-mcpu=cortex-m3", "-mthumb", "-g", "-nostdlib", "-T", linker, source, "-o", elf],
            [objcopy, "--only-keep-debug", elf, debug],
            [objcopy, "--strip-debug", elf],
            [objcopy, f"--add-gnu-debuglink={debug}", elf],
            [objcopy, *payload.OBJCOPY_ARGS, elf, self.raw],
        ]
        for command in commands:
            subprocess.run(command, check=True, capture_output=True, text=True)
        kwargs = {"symbols": {"APP": (elf, debug)}, "objcopy": objcopy}
        original = self.package(self.part(capacity=2048), **kwargs)
        with self.assertRaisesRegex(payload.ArtifactError, "load address 0x1000 differs from partition address 0x4000"):
            self.package(self.part(address="0x4000", capacity=2048), **kwargs)
        path = Path(original["manifest_path"])
        manifest = payload.load_artifact(path)
        self.assertEqual(manifest["partitions"][0]["objcopy"]["load_address"], "0x1000")
        packaged_debug = payload.manifest_file(path, manifest["partitions"][0]["symbols"]["debug_file"])
        self.assertEqual(packaged_debug.name, debug.name)
        self.assertEqual(packaged_debug.read_bytes(), debug.read_bytes())
        self.assertIn(b".debug_info", packaged_debug.read_bytes())

        # Recompute the manifest ID so the consumer must inspect ELF LMAs.
        manifest["partitions"][0]["address"] = "0x4000"
        manifest["partitions"][0]["objcopy"]["load_address"] = "0x4000"
        manifest["artifact_id"] = payload._artifact_id(manifest)
        path.write_text(json.dumps(manifest), encoding="utf-8")
        with self.assertRaises(payload.ArtifactError) as mismatch:
            payload.load_artifact(path)
        self.assertEqual(mismatch.exception.code, "ARTIFACT_MISMATCH")
        self.assertIn("0x1000", str(mismatch.exception))


if __name__ == "__main__":
    unittest.main()
