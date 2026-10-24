#!/usr/bin/env python3
"""Host-platform build check: loadconfig + clean build for the projects that
belong to one host platform, then assert the executable really exists.

Platform -> projects (the executable basename is asserted after the build):

  linux   -> project/linux/ehip-test          ehip_test
  macos   -> project/macos/ehip-test          ehip_test
  windows -> project/windows/ehip-test        ehip_test
             project/windows/co-ctx-test      co_ctx_test

The requested platform must match the host: the linux project links the epoll
backend, the windows project needs MinGW-w64 GCC (the toolchain files are GNU
only), and neither can be produced from macOS. CI calls this as the only step
of each host-* job:

    python3 tool/test/check_build.py --platform linux

Exit code is 0 only when every project of that platform configures, builds and
produces its executable. A build failure is fatal on its own (build.py returns
the real exit code), the executable assertion catches the case where a stale
artifact satisfies a broken link.
"""

from __future__ import annotations

import argparse
import dataclasses
import os
import platform as host_platform
import shutil
import subprocess
import sys
from pathlib import Path


# (defconfig path, expected executable basename)
PLATFORM_PROJECTS: dict[str, list[tuple[str, str]]] = {
    "linux": [
        ("project/linux/ehip-test/defconfig", "ehip_test"),
    ],
    "macos": [
        ("project/macos/ehip-test/defconfig", "ehip_test"),
    ],
    "windows": [
        ("project/windows/ehip-test/defconfig", "ehip_test"),
        ("project/windows/co-ctx-test/defconfig", "co_ctx_test"),
    ],
}


@dataclasses.dataclass
class BuildResult:
    defconfig_rel: str
    ok: bool
    error: str | None = None
    exe_path: Path | None = None


def host_platform_name() -> str:
    system = host_platform.system()
    if system == "Darwin":
        return "macos"
    if system == "Linux":
        return "linux"
    if system == "Windows":
        return "windows"
    return system.lower()


def _run(cmd: list[str], cwd: Path, timeout: int = 1800) -> tuple[int, str]:
    proc = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=timeout)
    return proc.returncode, (proc.stdout + proc.stderr)


def build_command(repo_root: Path) -> list[str]:
    """build.bat needs cmd.exe; build.sh runs directly on POSIX hosts."""
    if os.name == "nt":
        return ["cmd", "/c", str(repo_root / "build.bat")]
    return [str(repo_root / "build.sh")]


def find_executable(repo_root: Path, basename: str) -> Path | None:
    build_root = repo_root / "build"
    if not build_root.exists():
        return None
    for p in build_root.rglob(basename):
        if p.is_file() and os.access(p, os.X_OK):
            return p
    # Windows runners keep .exe; the glob above already matches it by basename
    # only if the caller passed "ehip_test", so try the suffix explicitly.
    for p in build_root.rglob(basename + ".exe"):
        if p.is_file():
            return p
    return None


def check_project(repo_root: Path, defconfig_rel: str, basename: str,
                  jobs: int, build_type: str, verbose: bool) -> BuildResult:
    defconfig = repo_root / defconfig_rel
    if not defconfig.exists():
        return BuildResult(defconfig_rel, False, f"defconfig not found: {defconfig}")

    cmd = build_command(repo_root)

    rc, out = _run(cmd + ["loadconfig", str(defconfig)], repo_root)
    if rc != 0:
        return BuildResult(defconfig_rel, False, f"loadconfig failed (rc={rc})\n{out[-2000:]}")

    # Clean configure/build so a leftover artifact cannot mask a broken link.
    shutil.rmtree(repo_root / "build", ignore_errors=True)

    rc, out = _run(cmd + ["build", build_type, "-j", str(jobs)], repo_root)
    if verbose:
        sys.stdout.write(out)
    if rc != 0:
        return BuildResult(defconfig_rel, False, f"build failed (rc={rc})\n{out[-4000:]}")

    exe = find_executable(repo_root, basename)
    if exe is None:
        return BuildResult(defconfig_rel, False, f"executable '{basename}' not produced")
    return BuildResult(defconfig_rel, True, exe_path=exe)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--repo-root", type=Path, default=Path(__file__).resolve().parent.parent.parent,
    )
    parser.add_argument(
        "--platform", default="auto",
        choices=["auto", "linux", "macos", "windows"],
        help="host platform to build for (default: this host)",
    )
    parser.add_argument("--build-type", default="Release",
                        choices=["Debug", "Release", "MinSizeRel", "RelWithDebInfo"])
    parser.add_argument("-j", "--jobs", type=int, default=4)
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args(argv)

    repo_root = args.repo_root.resolve()
    host = host_platform_name()
    target = host if args.platform == "auto" else args.platform

    if target not in PLATFORM_PROJECTS:
        sys.stderr.write(
            f"check_build: FAILED (unknown platform '{target}'; "
            f"known: {', '.join(sorted(PLATFORM_PROJECTS))})\n"
        )
        return 1
    if target != host:
        # Cross-linking a host backend is not a supported configuration.
        sys.stderr.write(
            f"check_build: FAILED (platform '{target}' cannot be built on host "
            f"'{host}'; run this check on a {target} machine or CI runner)\n"
        )
        return 1

    results: list[BuildResult] = []
    for defconfig_rel, basename in PLATFORM_PROJECTS[target]:
        sys.stdout.write(f"[check_build] {target}  {defconfig_rel}\n")
        sys.stdout.flush()
        result = check_project(repo_root, defconfig_rel, basename,
                               args.jobs, args.build_type, args.verbose)
        results.append(result)
        if result.ok:
            sys.stdout.write(f"  PASS  exe={result.exe_path.relative_to(repo_root)}\n")
        else:
            sys.stdout.write(f"  FAIL  {result.error}\n")

    failed = [r for r in results if not r.ok]
    if failed:
        sys.stderr.write("check_build: FAILED\n")
        for r in failed:
            sys.stderr.write(f"  {r.defconfig_rel}: {r.error}\n")
        return 1

    sys.stdout.write(f"check_build: PASS ({len(results)} {target} project(s))\n")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
