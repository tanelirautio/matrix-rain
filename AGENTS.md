# AGENTS.md

Guidance for coding agents working in this repository.

## Project Overview

Matrix Rain is a C++ project with a deterministic, testable simulation core and platform-specific rendering layers.

- `src/core/`: backend-agnostic simulation and supporting data structures.
- `src/sdl/`: SDL rendering and desktop/web platform code.
- `src/app/`: desktop app wiring and argument parsing.
- `tests/`: Catch2 tests for core and integration behavior.
- `android/`: Android app using JNI and native C++.
- `spec/`: planning and task-tracking documents.

Keep the simulation core portable. Do not introduce SDL, Android, JNI, or UI dependencies into `src/core`.

## Current Android Direction

The Android port should build `MatrixRainCore` as part of the Android native build and expose it through the `matrixrain` JNI shared library.

Use the docs in `spec/` as the source of intent:

- `spec/android-port-plan.md`
- `spec/android-port-todo.md`

The first Android implementation should keep rendering in Kotlin/Android `Canvas` and use native C++ only for simulation state and frame updates.

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
