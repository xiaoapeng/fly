"""Prepare the Python dependencies needed to build a FLY application."""

from importlib.util import find_spec
import os
import subprocess
import sys


DEPENDENCIES = ("kconfiglib", "requests")


def prepare_python(build_dir, *, output=None):
    """Return an application interpreter if the current one needs dependencies."""
    missing = [module for module in DEPENDENCIES if find_spec(module) is None]
    if not missing:
        return None

    venv = build_dir / ".venv"
    python = venv / ("Scripts/python.exe" if os.name == "nt" else "bin/python")
    probe = [str(python), "-B", "-c", f"import {', '.join(DEPENDENCIES)}"]
    if python.is_file() and subprocess.run(
        probe, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    ).returncode == 0:
        return python

    packages = " ".join(DEPENDENCIES)
    if os.environ.get("FLY_AUTO_SETUP_PY", "1") != "1":
        raise ValueError(
            f"Python dependencies missing: {', '.join(missing)}. "
            f'Install them with "{sys.executable}" -m pip install {packages}.'
        )

    if not python.is_file():
        print(f"Creating Python virtual environment at {venv}", file=output, flush=True)
        subprocess.run([sys.executable, "-m", "venv", str(venv)], stdout=output, check=True)

    print(f"Installing Python dependencies in {venv}", file=output, flush=True)
    subprocess.run(
        [str(python), "-m", "pip", "install", "--disable-pip-version-check", *DEPENDENCIES],
        stdout=output, check=True,
    )
    subprocess.run(probe, check=True)
    return python
