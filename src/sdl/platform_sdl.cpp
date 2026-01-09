#include "platform_sdl.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <tuple>

#include "grid.hpp"
#include "matrix_rain.hpp"
#include "renderer.hpp"
#include "sdl_block_renderer.hpp"
#include "sdl_ttf_glyph_renderer.hpp"

// ---- helpers ---------------------------------------------------------------

static std::tuple<int, int> snapToGlyphGrid(int width, int height, int cellW, int cellH) {
    int cols = std::max(1, width / cellW);
    int rows = std::max(1, height / cellH);
    return {cols * cellW, rows * cellH};
}

// ---- SdlPlatform ------------------------------------------------------------

SdlPlatform::SdlPlatform(const AppArgs& args) : m_args(args) {}

SdlPlatform::~SdlPlatform() = default;

void SdlPlatform::run() {
    // -------------------------------------------------------------------------
    // SDL init
    // -------------------------------------------------------------------------
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(SDL_GetError());
    }

    if (!TTF_Init()) {
        SDL_Quit();
        throw std::runtime_error("TTF_Init failed");
    }

    // -------------------------------------------------------------------------
    // Render configuration
    // -------------------------------------------------------------------------
    RenderConfig renderCfg{};
    renderCfg.cellWidthPx = m_args.cellWidthPx;
    renderCfg.cellHeightPx = m_args.cellHeightPx;

    // -------------------------------------------------------------------------
    // Window size & mode
    // -------------------------------------------------------------------------
    int windowW = m_args.width;
    int windowH = m_args.height;

    if (m_args.mode == StartMode::Windowed) {
        std::tie(windowW, windowH) = snapToGlyphGrid(windowW, windowH, renderCfg.cellWidthPx, renderCfg.cellHeightPx);
    }

    Uint64 windowFlags = (m_args.mode == StartMode::Windowed) ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN;
    windowFlags |= SDL_WINDOW_HIDDEN;

    SDL_Window* window = SDL_CreateWindow("SDL3 Matrix Rain", windowW, windowH, windowFlags);

    if (!window) {
        TTF_Quit();
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(window, nullptr);
    if (!sdlRenderer) {
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    // -------------------------------------------------------------------------
    // Matrix rain configuration helper
    // -------------------------------------------------------------------------
    auto makeConfigFromPixels = [&](int pixelW, int pixelH) {
        auto [cols, rows] = computeGridFromPixels(pixelW, pixelH, renderCfg.cellWidthPx, renderCfg.cellHeightPx);

        matrix_rain::Config cfg{};
        cfg.columns = cols;
        cfg.rows = rows;
        cfg.spawnRatePerColumn = 0.3f;
        cfg.minSpeedRowsPerSecond = 1.0f;
        cfg.maxSpeedRowsPerSecond = 10.0f;
        cfg.fadeRatePerSecond = 1.0f;
        cfg.useFixedSeed = m_args.useFixedSeed;
        cfg.rngSeed = m_args.rngSeed;

        return cfg;
    };

    int pixelW = 0, pixelH = 0;
    SDL_GetWindowSizeInPixels(window, &pixelW, &pixelH);

    matrix_rain::MatrixRain rain(makeConfigFromPixels(pixelW, pixelH));

    // -------------------------------------------------------------------------
    // Renderer backend
    // -------------------------------------------------------------------------
    const float fontPtSize = m_args.fontSizeProvided ? m_args.fontSizePt : renderCfg.cellHeightPx * 0.75f;
    auto renderer = std::make_unique<SdlTtfGlyphRenderer>(
        sdlRenderer, renderCfg, "assets/fonts/NotoSansMonoCJKJP-Regular.otf", fontPtSize, m_args.debugDumps);

    SDL_ShowWindow(window);

    // -------------------------------------------------------------------------
    // Main loop
    // -------------------------------------------------------------------------
    bool running = true;
    SDL_Event e;

    using clock = std::chrono::steady_clock;
    auto lastTime = clock::now();

    while (running) {
        // Delta time
        auto now = clock::now();
        std::chrono::duration<float> delta = now - lastTime;
        lastTime = now;
        float dt = std::clamp(delta.count(), 0.0f, 0.1f);

        // Events
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                SDL_GetWindowSizeInPixels(window, &pixelW, &pixelH);
                rain = matrix_rain::MatrixRain(makeConfigFromPixels(pixelW, pixelH));
                renderer->onResizePixels(pixelW, pixelH);
                break;

            default:
                break;
            }
        }

        // Simulation + rendering
        rain.update(dt);
        renderer->render(rain);
        renderer->present();
    }

    // -------------------------------------------------------------------------
    // Shutdown
    // -------------------------------------------------------------------------
    renderer.reset();
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}
