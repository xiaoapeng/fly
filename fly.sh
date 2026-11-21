#!/usr/bin/env sh
set -eu

sdk_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
for python in "${FLY_PYTHON:-}" "$sdk_dir/.venv/bin/python" python3 python; do
    if [ -n "$python" ] && command -v "$python" >/dev/null 2>&1 &&
        "$python" -c 'import sys; sys.exit(sys.version_info < (3, 8))' >/dev/null 2>&1; then
        exec "$python" -B "$sdk_dir/tool/python/project_new.py" "$@"
    fi
done
printf '%s\n' 'Python 3.8 or later is required to generate a FLY project.' >&2
exit 1
