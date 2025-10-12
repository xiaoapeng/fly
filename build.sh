#!/bin/bash

script_dir=$(cd "$(dirname "$0")"; pwd)
script_path="$script_dir/$(basename "$0")"

# 如果.venv存在，则source .venv/bin/activate
venv_dir="$script_dir/.venv"
if [ -d "$venv_dir" ]; then
    source "$venv_dir/bin/activate"
fi

python3 ${script_dir}/tool/python/build.py ${script_dir} $@

