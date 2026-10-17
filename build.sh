#!/usr/bin/env bash
set -euo pipefail

preset="${1:-release-linux}"

cmake --preset "$preset"
cmake --build --preset "$preset"
cmake --install ".build/$preset" --prefix ./out --component runtime
