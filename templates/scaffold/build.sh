#!/usr/bin/env sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
try_python() {
    [ -n "$1" ] && command -v "$1" >/dev/null 2>&1 &&
        "$1" -c 'import sys; sys.exit(sys.version_info < (3, 8))' >/dev/null 2>&1
}

find_python() {
    for python in "${FLY_PYTHON:-}" python3 python; do
        if try_python "$python"; then
            printf '%s\n' "$python"
            return
        fi
    done

    # These interpreter hints only bootstrap Python. Python selects and checks the SDK.
    stored_hint=
    if [ -f "$project_dir/.fly-root" ]; then
        IFS= read -r stored_hint < "$project_dir/.fly-root" || true
    fi
    for hint in "${FLY_ROOT:-}" "$project_dir/fly" "$stored_hint"; do
        case "$hint" in
            '') continue ;;
            /*) ;;
            *) hint=$project_dir/$hint ;;
        esac
        python=$hint/.venv/bin/python
        if try_python "$python"; then
            printf '%s\n' "$python"
            return
        fi
    done
    printf '%s\n' 'Python 3.8 or later is required to run FLY.' >&2
    return 1
}

python=$(find_python) || exit 1
exec "$python" -B "$project_dir/tool/python/setup_fly.py" --build "$@"
