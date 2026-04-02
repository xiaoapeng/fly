#!/usr/bin/env bash

set -euo pipefail

script_dir=$(cd "$(dirname "$0")" && pwd)
build_py="$script_dir/tool/python/build.py"
venv_dir="$script_dir/.venv"

require_module() {
    local py_bin="$1"
    local module="$2"
    "$py_bin" -c "import $module" >/dev/null 2>&1
}

find_python() {
    local candidate

    # Allow overriding the Python interpreter with an environment variable.
    if [ -n "${FLY_PYTHON:-}" ] && [ -x "${FLY_PYTHON}" ]; then
        echo "${FLY_PYTHON}"
        return 0
    fi

    # Prefer project-local virtualenv to avoid polluting system Python.
    if [ -x "$venv_dir/bin/python" ]; then
        echo "$venv_dir/bin/python"
        return 0
    fi

    for candidate in python3 python; do
        if command -v "$candidate" >/dev/null 2>&1; then
            command -v "$candidate"
            return 0
        fi
    done

    return 1
}

ensure_venv_and_deps() {
    local bootstrap_py="$1"
    local use_venv=1

    if [ ! -x "$venv_dir/bin/python" ]; then
        echo "[build.sh] Creating virtual environment at $venv_dir ..."
        if ! "$bootstrap_py" -m venv "$venv_dir" >/dev/null 2>&1; then
            use_venv=0
            echo "[build.sh] Warning: failed to create .venv, fallback to user site-packages."
        fi
    fi

    if [ "$use_venv" = "1" ] && [ -x "$venv_dir/bin/python" ]; then
        # shellcheck disable=SC1091
        source "$venv_dir/bin/activate"
    fi

    echo "[build.sh] Installing required Python packages ..."
    # Improve compatibility across macOS/Linux permission/cache behaviors.
    if [ "$use_venv" = "1" ] && [ -x "$venv_dir/bin/python" ]; then
        PIP_NO_CACHE_DIR=1 python -m pip install --upgrade --disable-pip-version-check pip >/dev/null
        PIP_NO_CACHE_DIR=1 python -m pip install --disable-pip-version-check kconfiglib requests >/dev/null
    else
        PIP_NO_CACHE_DIR=1 "$bootstrap_py" -m pip install --user --disable-pip-version-check kconfiglib requests >/dev/null
    fi
}

python_bin=$(find_python || true)
if [ -z "${python_bin}" ]; then
    echo "[build.sh] Error: no usable python interpreter found (tried FLY_PYTHON/.venv/python/python3/python)." >&2
    exit 1
fi

if ! require_module "$python_bin" kconfiglib; then
    echo "[build.sh] Missing dependency: kconfiglib (current python: $python_bin)"

    if [ "${FLY_AUTO_SETUP_PY:-1}" = "1" ]; then
        ensure_venv_and_deps "$python_bin"
        if [ -x "$venv_dir/bin/python" ]; then
            python_bin="$venv_dir/bin/python"
        fi
        if ! require_module "$python_bin" kconfiglib; then
            echo "[build.sh] Error: kconfiglib install failed for $python_bin" >&2
            echo "[build.sh] Try manually: $python_bin -m pip install kconfiglib requests" >&2
            exit 1
        fi
    else
        echo "[build.sh] Auto setup disabled. Please run:" >&2
        echo "  $python_bin -m pip install kconfiglib requests" >&2
        echo "or enable auto setup by exporting FLY_AUTO_SETUP_PY=1" >&2
        exit 1
    fi
fi

exec "$python_bin" "$build_py" "$script_dir" "$@"
