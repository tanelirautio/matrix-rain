#include <catch2/catch_test_macros.hpp>

#include "sdl_ttf_glyph_renderer.hpp"
#include "renderer.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace {
    struct SdlScope {
        SdlScope() {
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                throw std::runtime_error(SDL_GetError());
            }
            if (!TTF_Init()) {
                SDL_Quit();
                throw std::runtime_error("TTF_Init failed");
            }
        }
        ~SdlScope() {
            TTF_Quit();
            SDL_Quit();
        }
    };

    SDL_Window* createHiddenWindow() {
        return SDL_CreateWindow("MatrixRainTests", 64, 64, SDL_WINDOW_HIDDEN);
    }

    std::string assetsFontPath() {
        std::filesystem::path p = MATRIX_RAIN_ASSETS_DIR;
        p /= "fonts";
        p /= "NotoSansMonoCJKJP-Regular.otf";
        return p.string();
    }
} // namespace

TEST_CASE("Assets: default font exists in source tree") {
    std::filesystem::path p = MATRIX_RAIN_ASSETS_DIR;
    p /= "fonts";
    p /= "NotoSansMonoCJKJP-Regular.otf";
    REQUIRE(std::filesystem::exists(p));
}

TEST_CASE("Assets: relative assets path exists in working directory") {
    std::filesystem::path p = "assets/fonts/NotoSansMonoCJKJP-Regular.otf";
    REQUIRE(std::filesystem::exists(p));
}

TEST_CASE("SDL renderer: atlas creation succeeds with default font") {
    SdlScope sdl;
    SDL_Window* window = createHiddenWindow();
    REQUIRE(window != nullptr);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    REQUIRE(renderer != nullptr);

    RenderConfig cfg{};
    cfg.cellWidthPx = 32;
    cfg.cellHeightPx = 32;

    REQUIRE_NOTHROW(SdlTtfGlyphRenderer(renderer, cfg, assetsFontPath(), cfg.cellHeightPx * 0.75f, false));

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

TEST_CASE("SDL renderer: missing font fails fast") {
    SdlScope sdl;
    SDL_Window* window = createHiddenWindow();
    REQUIRE(window != nullptr);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    REQUIRE(renderer != nullptr);

    RenderConfig cfg{};
    cfg.cellWidthPx = 32;
    cfg.cellHeightPx = 32;

    REQUIRE_THROWS(SdlTtfGlyphRenderer(renderer, cfg, "missing_font.ttf", cfg.cellHeightPx * 0.75f, false));

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

TEST_CASE("SDL renderer: atlas pack failure exits with error") {
    SdlScope sdl;
    SDL_Window* window = createHiddenWindow();
    REQUIRE(window != nullptr);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    REQUIRE(renderer != nullptr);

    RenderConfig cfg{};
    cfg.cellWidthPx = 512;
    cfg.cellHeightPx = 512;

    REQUIRE_THROWS(SdlTtfGlyphRenderer(renderer, cfg, assetsFontPath(), cfg.cellHeightPx * 0.75f, false));

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}
