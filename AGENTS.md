# AGENTS.md

Guidance for coding agents working in this repository.

## Project Overview

Matrix Rain is a C++ project with a deterministic, testable simulation core and platform-specific rendering layers.

- `src/core/`: backend-agnostic simulation and supporting data structures.
- `src/sdl/`: SDL rendering and desktop/web platform code.
- `src/app/`: desktop app wiring and argument parsing.
- `tests/`: Catch2 tests for core and integration behavior.
- `android/`: Android app, Kotlin Canvas rendering, and JNI/native C++ build.

Keep the simulation core portable. Do not introduce SDL, Android, JNI, or UI dependencies into `src/core`.

## Android Implementation

The Android app builds `MatrixRainCore` as part of the Android native build and exposes it through the `matrixrain` JNI shared library.

Kotlin owns lifecycle and rendering:

- `MatrixRainView` renders glyphs with Android `Canvas` and `Paint`.
- `MatrixRainNativeSession` owns the native handle and makes cleanup idempotent.
- Native C++ owns simulation state, frame updates, resizing, and cell reads only.

Keep Android rendering in Kotlin/Android unless a change explicitly introduces a native rendering backend.

## Build And Test

Desktop configure/build on Windows:

```powershell
powershell -File scripts\configure.ps1
cmake --build build --config Debug
```

Desktop tests:

```powershell
powershell -File scripts\test.ps1
```

Android build:

```powershell
cd android
.\gradlew.bat assembleDebug
```

Android unit tests:

```powershell
cd android
.\gradlew.bat :app:testDebugUnitTest
```

Android instrumented test APK:

```powershell
cd android
.\gradlew.bat :app:assembleDebugAndroidTest
```

Run the smallest relevant verification for the change. For documentation-only edits, no build is required.

## CMake Notes

- Keep `MatrixRainCore` available without platform dependencies.
- Desktop-only targets may depend on SDL3, SDL3_ttf, and Catch2.
- Android native builds should not require SDL3, SDL3_ttf, or Catch2.
- Prefer target-specific include directories and link libraries.

## Code Style

- C++ standard: C++20.
- Follow the existing `.clang-format`.
- Prefer small, explicit interfaces between platform code and core code.
- Avoid broad refactors while making platform-specific changes.
- Keep comments useful and sparse.

## Git Hygiene

The working tree may contain user changes. Do not revert unrelated changes. Check `git status --short` before and after significant edits when practical.
