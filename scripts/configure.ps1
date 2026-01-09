param(
  [string]$Toolchain = "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
)

$ErrorActionPreference = "Stop"
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$Toolchain