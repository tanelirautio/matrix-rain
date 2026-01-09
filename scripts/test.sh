#!/usr/bin/env bash
set -euo pipefail

config="${CONFIG:-}"
if [[ -n "$config" ]]; then
  cmake --build build --config "$config"
  ctest --test-dir build -C "$config" --output-on-failure
else
  cmake --build build
  ctest --test-dir build --output-on-failure
fi
