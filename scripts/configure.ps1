param(
  [string]$Toolchain = ""
)

$ErrorActionPreference = "Stop"

function Get-VcpkgToolchainCandidates {
  if ($env:VCPKG_ROOT) {
    Join-Path $env:VCPKG_ROOT "scripts/buildsystems/vcpkg.cmake"
  }

  $vcpkgCommand = Get-Command vcpkg.exe -ErrorAction SilentlyContinue
  if ($vcpkgCommand) {
    Join-Path (Split-Path $vcpkgCommand.Source -Parent) "scripts/buildsystems/vcpkg.cmake"
  }

  "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
  Get-ChildItem "C:/Program Files/Microsoft Visual Studio" -Directory -ErrorAction SilentlyContinue |
    ForEach-Object {
      Get-ChildItem $_.FullName -Directory -ErrorAction SilentlyContinue |
        ForEach-Object {
          Join-Path $_.FullName "VC/vcpkg/scripts/buildsystems/vcpkg.cmake"
        }
    }
}

if ($Toolchain) {
  if (-not (Test-Path $Toolchain)) {
    throw "vcpkg toolchain file not found: $Toolchain"
  }
} else {
  $Toolchain = Get-VcpkgToolchainCandidates |
    Select-Object -Unique |
    Where-Object { Test-Path $_ } |
    Select-Object -First 1

  if (-not $Toolchain) {
    throw "vcpkg toolchain file not found. Set VCPKG_ROOT or pass -Toolchain <path>."
  }
}

cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$Toolchain
