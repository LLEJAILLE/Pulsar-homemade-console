#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"

if [[ ! -f "build/CMakeCache.txt" ]]; then
    cmake -S . -B build
fi

cmake --build build

./build/Pulsar