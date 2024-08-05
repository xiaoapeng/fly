#!/bin/bash

script_dir=$(cd "$(dirname "$0")"; pwd)
script_path="$script_dir/$(basename "$0")"

python3 ${script_dir}/tool/python/build.py ${script_dir} $@

