param(
  [ValidateSet("Debug","Release")]
  [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

cmake --build build --config $Config --target MatrixRainTests
ctest --test-dir build -C $Config --output-on-failure