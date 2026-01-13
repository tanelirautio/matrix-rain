cmake_minimum_required(VERSION 3.21)
project(MatrixRainSDL3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies (via vcpkg or Emscripten ports)
if (EMSCRIPTEN)
    set(MATRIX_RAIN_SDL_LIBS "")
else()
    find_package(SDL3 CONFIG REQUIRED)
    find_package(SDL3_ttf CONFIG REQUIRED)
    find_package(Catch2 3 CONFIG REQUIRED)
    set(MATRIX_RAIN_SDL_LIBS SDL3::SDL3 SDL3_ttf::SDL3_ttf)
endif()

# ---------------------------
# Core library (backend-agnostic)
# ---------------------------
add_library(MatrixRainCore
    src/core/matrix_rain.cpp
    src/core/rect_packer.cpp
    src/core/glyph_atlas.cpp
    src/core/debug_image_io.cpp
)

target_include_directories(MatrixRainCore PUBLIC src/core)

# Warnings (apply to core so both app + tests inherit)
if (MSVC)
    target_compile_options(MatrixRainCore PRIVATE /W4 /permissive-)
else()
    target_compile_options(MatrixRainCore PRIVATE -Wall -Wextra -pedantic)
endif()

# ---------------------------
# App (SDL-specific)
# ---------------------------
add_executable(MatrixRain
    src/main.cpp
    src/app/app.cpp
    src/app/platform.cpp
    src/sdl/platform_sdl.cpp
    src/app/args.cpp
    src/sdl/sdl_block_renderer.cpp
    src/sdl/sdl_ttf_glyph_renderer.cpp
    src/sdl/sdl_ttf_rasterizer.cpp
)

target_include_directories(MatrixRain
    PRIVATE
        src/app
        src/sdl
)

target_link_libraries(MatrixRain
    PRIVATE
        MatrixRainCore
        ${MATRIX_RAIN_SDL_LIBS}
)

if (EMSCRIPTEN)
    set_target_properties(MatrixRain PROPERTIES SUFFIX ".html" OUTPUT_NAME "index")
    target_include_directories(MatrixRain
        PRIVATE
            ${CMAKE_SYSROOT}/include/SDL2
    )
    target_compile_options(MatrixRain
        PRIVATE
            "SHELL:-sUSE_SDL=2"
            "SHELL:-sUSE_SDL_TTF=2"
    )
    target_link_options(MatrixRain
        PRIVATE
            "SHELL:-sUSE_SDL=2"
            "SHELL:-sUSE_SDL_TTF=2"
            "SHELL:-sALLOW_MEMORY_GROWTH=1"
            "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
            "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/assets/web/shell.html"
    )
else()
    # Copy runtime DLLs next to the exe (Windows)
    add_custom_command(TARGET MatrixRain POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              $<TARGET_RUNTIME_DLLS:MatrixRain>
              $<TARGET_FILE_DIR:MatrixRain>
      COMMAND_EXPAND_LISTS
    )

    # Copy runtime assets next to the executable.
    add_custom_command(TARGET MatrixRain POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
              ${CMAKE_SOURCE_DIR}/assets
              $<TARGET_FILE_DIR:MatrixRain>/assets
    )

    # Keep assets available from the build directory for tests and local runs.
    file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

# ---------------------------
# Tests
# ---------------------------
if (NOT EMSCRIPTEN)
    enable_testing()

    add_executable(MatrixRainTests
        tests/test_matrix_rain.cpp
        tests/test_rect_packer.cpp
        tests/test_rng.cpp
        tests/test_glyph_atlas.cpp
        tests/test_args.cpp
        tests/test_platform_grid.cpp
        tests/test_sdl_integration.cpp
        src/app/args.cpp
        src/sdl/sdl_ttf_glyph_renderer.cpp
        src/sdl/sdl_ttf_rasterizer.cpp
    )

    target_link_libraries(MatrixRainTests
        PRIVATE
            MatrixRainCore
            Catch2::Catch2WithMain
            SDL3::SDL3
            SDL3_ttf::SDL3_ttf
    )

    target_include_directories(MatrixRainTests
        PRIVATE
            src/app
            src/sdl
    )

    target_compile_definitions(MatrixRainTests
        PRIVATE
            MATRIX_RAIN_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
    )

    add_custom_command(TARGET MatrixRainTests POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
              ${CMAKE_SOURCE_DIR}/assets
              $<TARGET_FILE_DIR:MatrixRainTests>/assets
    )

    include(Catch)
    catch_discover_tests(MatrixRainTests)
endif()
