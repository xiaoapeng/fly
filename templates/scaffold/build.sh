#!/usr/bin/env sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
find_python() {
    sdk_python=
    if [ -n "$fly_dir" ]; then
        sdk_python=$fly_dir/.venv/bin/python
    fi
    for python in "${FLY_PYTHON:-}" "$sdk_python" python3 python; do
        if [ -n "$python" ] && command -v "$python" >/dev/null 2>&1 &&
            "$python" -c 'import sys; sys.exit(sys.version_info < (3, 8))' >/dev/null 2>&1; then
            printf '%s\n' "$python"
            return
        fi
    done
    printf '%s\n' 'Python 3.8 or later is required to run FLY.' >&2
    return 1
}

if [ -n "${FLY_ROOT:-}" ]; then
    fly_dir=$FLY_ROOT
elif [ -e "$project_dir/fly" ] || [ -L "$project_dir/fly" ]; then
    fly_dir=$project_dir/fly
elif [ -f "$project_dir/.fly-root" ]; then
    fly_dir=
    IFS= read -r fly_dir < "$project_dir/.fly-root" || true
else
    fly_dir=
fi

if [ "${1:-}" = setup ]; then
    shift
    python=$(find_python) || exit 1
    exec "$python" -B "$project_dir/tool/python/setup_fly.py" "$@"
fi

if [ -z "$fly_dir" ] || [ ! -f "$fly_dir/tool/python/build.py" ]; then
    printf '%s\n' 'Could not locate FLY. Run ./build.sh setup to install the SDK from fly.lock, or set FLY_ROOT.' >&2
    exit 1
fi
if [ ! -f "$fly_dir/cmake/fly-configure.cmake" ]; then
    printf '%s\n' 'This app needs a FLY SDK with application-root CMake. Update fly.lock and run setup, or set FLY_ROOT.' >&2
    exit 1
fi

python=$(find_python) || exit 1
"$python" -B "$project_dir/tool/python/setup_fly.py" --check --sdk-dir "$fly_dir"
exec "$python" -B "$fly_dir/tool/python/build.py" --project-dir "$project_dir" "$@"
