# Matrix Rain (SDL3)

Matrix Rain is a small C++/SDL3 project that renders a classic "digital rain"
effect with a deterministic, testable simulation core and an atlas-based glyph
renderer.

## Features

- Atlas-based glyph rendering (SDL3_ttf on desktop)
- Brighter head, fading trail
- HiDPI-aware grid sizing based on pixel size
- Deterministic simulation via fixed RNG seed
- Core simulation independent of SDL

## Build (Windows + vcpkg)

This project uses a vcpkg manifest (see `vcpkg.json`).

Configure:
```
powershell -File scripts\configure.ps1
```

Build:
```
cmake --build build --config Debug
```

Run:
```
build\Debug\MatrixRain.exe
```

## Cross-platform helpers

On macOS/Linux you can use the bash scripts:
```
./scripts/configure.sh
./scripts/test.sh
```

You can also use CMake presets:
```
cmake --preset vcpkg
cmake --build --preset build
ctest --preset test
```

On Windows (multi-config), use the Debug presets:
```
cmake --preset vcpkg
cmake --build --preset build-debug
ctest --preset test-debug
```

## Web (Emscripten)

Live demo: https://tanelirautio.github.io/matrix-rain/

Prereqs:
- Install emsdk and run `emsdk_env` so `emcc` is on PATH.
- Ensure `EMSDK` is set (used by the CMake preset).
Note: the web build uses Emscripten's SDL2 + SDL2_ttf ports.

Configure + build:
```
cmake --preset emscripten
cmake --build --preset build-emscripten
```

Run locally (serve the build output):
```
cd build-emscripten
python -m http.server
```
Open `http://localhost:8000/index.html`.

## Deploy (static hosting)

The Emscripten build now outputs `index.html` plus `index.js`, `index.wasm`, and `index.data`.
You can upload the contents of `build-emscripten/` to any static host and it will serve the
Matrix Rain page at the site root.

For Render.com:
- Create a Static Site.
- Use the `build-emscripten/` folder as the publish directory (build locally, then deploy).

## Dependencies and discovery

This project uses `find_package(SDL3 CONFIG REQUIRED)` and
`find_package(SDL3_ttf CONFIG REQUIRED)`. That means CMake must be able to
locate SDL3 and SDL3_ttf config files.
For the web build, Emscripten uses SDL2 + SDL2_ttf ports instead of these packages.

If CMake fails to configure with "could not find SDL3", you need to point it at
your package manager's install prefix:

- vcpkg (recommended):
  ```
  cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
  ```
- Homebrew (macOS):
  ```
  cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew
  ```
  (Use `/usr/local` on Intel Macs.)
- System packages (Linux):
  ```
  cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr
  ```
  Adjust the path if your distro installs to a different prefix.

## Tests

```
powershell -File scripts\test.ps1
```

## Command-line options

```
  --fullscreen, -f               Start in fullscreen mode
  --windowed, -w                 Start in windowed mode
  --width <pixels>, -W <pixels>  Set window width (windowed mode)
  --height <pixels>, -H <pixels> Set window height (windowed mode)
  --cell <pixels>                Set square cell size in pixels
  --cell-width <pixels>          Set cell width in pixels
  --cell-height <pixels>         Set cell height in pixels
  --fontSize <points>            Override font size in points
  --seed <uint>                  Use fixed RNG seed
  --help, -h                     Show help
```

## Project layout

```
src/
  main.cpp        Entry point
  app/            App wiring and argument parsing
  core/           Simulation + atlas packing (SDL-free)
  sdl/            SDL3 rendering and platform layer
tests/            Core tests (Catch2)
assets/           Fonts and licenses
```

## Notes

- The default font is `assets/fonts/NotoSansMonoCJKJP-Regular.otf`.
- Press `Esc` to exit.
- The bundled font is licensed under the SIL Open Font License; see `assets/licenses/OFL.txt`.
