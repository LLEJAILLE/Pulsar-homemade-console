#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"

if [[ ! -f "build/CMakeCache.txt" ]]; then
    cmake -B build -DCMAKE_BUILD_TYPE=Release
fi

cmake --build build -j4

./build/Pulsar