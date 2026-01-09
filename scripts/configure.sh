#!/usr/bin/env bash
set -euo pipefail

cmake_args=()
if [[ -n "${VCPKG_ROOT:-}" ]]; then
  cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
fi

cmake -S . -B build "${cmake_args[@]}" "$@"
