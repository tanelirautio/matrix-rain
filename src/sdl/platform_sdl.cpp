#include "platform_sdl.hpp"

#include "sdl_compat.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

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

static matrix_rain::Config makeConfigFromPixels(
    int pixelW, int pixelH, const RenderConfig &renderCfg, const AppArgs &args) {
    auto [cols, rows] = computeGridFromPixels(pixelW, pixelH, renderCfg.cellWidthPx, renderCfg.cellHeightPx);

    matrix_rain::Config cfg{};
    cfg.columns = cols;
    cfg.rows = rows;
    cfg.spawnRatePerColumn = 0.3f;
    cfg.minSpeedRowsPerSecond = 1.0f;
    cfg.maxSpeedRowsPerSecond = 10.0f;
    cfg.fadeRatePerSecond = 1.0f;
    cfg.useFixedSeed = args.useFixedSeed;
    cfg.rngSeed = args.rngSeed;

    return cfg;
}

static void queryRenderPixels(SDL_Window *window, SDL_Renderer *renderer, int *pixelW, int *pixelH) {
#if MATRIX_RAIN_SDL2 && defined(__EMSCRIPTEN__)
    int w = 0;
    int h = 0;
    if (emscripten_get_canvas_element_size("#canvas", &w, &h) == EMSCRIPTEN_RESULT_SUCCESS && w > 0 && h > 0) {
        *pixelW = w;
        *pixelH = h;
        return;
    }
#endif
    matrix_rain_get_window_size_pixels(window, pixelW, pixelH);
}

#if MATRIX_RAIN_SDL2 && defined(__EMSCRIPTEN__)
static bool snapCanvasToGrid(SDL_Window *window, const RenderConfig &renderCfg, int *pixelW, int *pixelH) {
    double cssW = 0.0;
    double cssH = 0.0;
    if (emscripten_get_element_css_size("body", &cssW, &cssH) != EMSCRIPTEN_RESULT_SUCCESS) {
        return false;
    }
    const int cols = std::max(1, static_cast<int>(cssW) / renderCfg.cellWidthPx);
    const int rows = std::max(1, static_cast<int>(cssH) / renderCfg.cellHeightPx);
    const int snappedW = cols * renderCfg.cellWidthPx;
    const int snappedH = rows * renderCfg.cellHeightPx;

    int currentW = 0;
    int currentH = 0;
    emscripten_get_canvas_element_size("#canvas", &currentW, &currentH);

    bool changed = false;
    if (currentW != snappedW || currentH != snappedH) {
        emscripten_set_canvas_element_size("#canvas", snappedW, snappedH);
        changed = true;
    }
    if (static_cast<int>(cssW) != snappedW || static_cast<int>(cssH) != snappedH) {
        emscripten_set_element_css_size("#canvas", snappedW, snappedH);
    }
    if (window && (currentW != snappedW || currentH != snappedH)) {
        SDL_SetWindowSize(window, snappedW, snappedH);
    }
    if (changed) {
        *pixelW = snappedW;
        *pixelH = snappedH;
    }
    return changed;
}
#endif

struct LoopState {
    AppArgs args;
    RenderConfig renderCfg{};
    SDL_Window *window = nullptr;
    SDL_Renderer *sdlRenderer = nullptr;
    std::unique_ptr<IRenderer> renderer;
    matrix_rain::MatrixRain rain;
    int pixelW = 0;
    int pixelH = 0;
    bool running = true;
    std::chrono::steady_clock::time_point lastTime;

    LoopState(const AppArgs &inArgs, RenderConfig cfg, SDL_Window *win, SDL_Renderer *rendererIn, int w, int h)
        : args(inArgs),
          renderCfg(cfg),
          window(win),
          sdlRenderer(rendererIn),
          rain(makeConfigFromPixels(w, h, cfg, inArgs)),
          pixelW(w),
          pixelH(h),
          running(true),
          lastTime(std::chrono::steady_clock::now()) {}
};

static void shutdown(LoopState &state) {
    state.renderer.reset();
    if (state.sdlRenderer) {
        SDL_DestroyRenderer(state.sdlRenderer);
        state.sdlRenderer = nullptr;
    }
    if (state.window) {
        SDL_DestroyWindow(state.window);
        state.window = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}

static bool stepFrame(LoopState &state) {
    using clock = std::chrono::steady_clock;
    auto now = clock::now();
    std::chrono::duration<float> delta = now - state.lastTime;
    state.lastTime = now;
    float dt = std::clamp(delta.count(), 0.0f, 0.1f);

#if MATRIX_RAIN_SDL2 && defined(__EMSCRIPTEN__)
    int snappedW = state.pixelW;
    int snappedH = state.pixelH;
    if (snapCanvasToGrid(state.window, state.renderCfg, &snappedW, &snappedH)) {
        state.pixelW = snappedW;
        state.pixelH = snappedH;
        state.rain = matrix_rain::MatrixRain(
            makeConfigFromPixels(state.pixelW, state.pixelH, state.renderCfg, state.args));
        state.renderer->onResizePixels(state.pixelW, state.pixelH);
    }
#endif

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
#if MATRIX_RAIN_SDL2
        switch (e.type) {
        case SDL_QUIT:
            state.running = false;
            break;

        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                state.running = false;
            }
            break;

        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                matrix_rain_get_window_size_pixels(state.window, &state.pixelW, &state.pixelH);
                state.rain = matrix_rain::MatrixRain(
                    makeConfigFromPixels(state.pixelW, state.pixelH, state.renderCfg, state.args));
                state.renderer->onResizePixels(state.pixelW, state.pixelH);
            }
            break;

        default:
            break;
        }
#else
        switch (e.type) {
        case SDL_EVENT_QUIT:
            state.running = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (e.key.key == SDLK_ESCAPE) {
                state.running = false;
            }
            break;

        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            queryRenderPixels(state.window, state.sdlRenderer, &state.pixelW, &state.pixelH);
            state.rain = matrix_rain::MatrixRain(
                makeConfigFromPixels(state.pixelW, state.pixelH, state.renderCfg, state.args));
            state.renderer->onResizePixels(state.pixelW, state.pixelH);
            break;

        default:
            break;
        }
#endif
    }

    if (!state.running) {
        return false;
    }

    state.rain.update(dt);
    state.renderer->render(state.rain);
    state.renderer->present();
    return true;
}

#ifdef __EMSCRIPTEN__
static void mainLoop(void *userData) {
    auto *state = static_cast<LoopState *>(userData);
    if (!stepFrame(*state)) {
        shutdown(*state);
        delete state;
        emscripten_cancel_main_loop();
    }
}
#endif

// ---- SdlPlatform ------------------------------------------------------------

SdlPlatform::SdlPlatform(const AppArgs& args) : m_args(args) {}

SdlPlatform::~SdlPlatform() = default;

void SdlPlatform::run() {
    // -------------------------------------------------------------------------
    // SDL init
    // -------------------------------------------------------------------------
    if (!matrix_rain_sdl_init_video()) {
        throw std::runtime_error(SDL_GetError());
    }

    if (!matrix_rain_ttf_init()) {
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

    SDL_Window* window = matrix_rain_create_window("Matrix Rain", windowW, windowH, windowFlags);

    if (!window) {
        TTF_Quit();
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    SDL_Renderer* sdlRenderer = matrix_rain_create_renderer(window);
    if (!sdlRenderer) {
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
#ifdef __EMSCRIPTEN__
    if (SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND) != 0) {
        std::fprintf(stderr, "MatrixRain web: SDL_SetRenderDrawBlendMode failed: %s\n", SDL_GetError());
        std::fflush(stderr);
    }
#endif

    int pixelW = 0, pixelH = 0;
    queryRenderPixels(window, sdlRenderer, &pixelW, &pixelH);

#ifdef __EMSCRIPTEN__
    auto *state = new LoopState(m_args, renderCfg, window, sdlRenderer, pixelW, pixelH);
    const float fontPtSize = m_args.fontSizeProvided ? m_args.fontSizePt : renderCfg.cellHeightPx * 0.75f;
    state->renderer = std::make_unique<SdlTtfGlyphRenderer>(
        sdlRenderer, renderCfg, "assets/fonts/NotoSansMonoCJKJP-Regular.otf", fontPtSize, m_args.debugDumps);
    snapCanvasToGrid(window, renderCfg, &state->pixelW, &state->pixelH);
    state->rain = matrix_rain::MatrixRain(
        makeConfigFromPixels(state->pixelW, state->pixelH, state->renderCfg, state->args));
    state->renderer->onResizePixels(state->pixelW, state->pixelH);
    SDL_ShowWindow(window);
    emscripten_set_main_loop_arg(&mainLoop, state, 0, true);
    return;
#else
    LoopState state(m_args, renderCfg, window, sdlRenderer, pixelW, pixelH);
    const float fontPtSize = m_args.fontSizeProvided ? m_args.fontSizePt : renderCfg.cellHeightPx * 0.75f;
    const char *fontPath = "assets/fonts/NotoSansMonoCJKJP-Regular.otf";
    state.renderer = std::make_unique<SdlTtfGlyphRenderer>(
        sdlRenderer, renderCfg, fontPath, fontPtSize, m_args.debugDumps);
    SDL_ShowWindow(window);
    while (stepFrame(state)) {
    }
    shutdown(state);
#endif
}
