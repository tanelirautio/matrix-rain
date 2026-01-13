#pragma once
#include "renderer.hpp"
#include "sdl_compat.hpp"

class SdlBlockRenderer final : public IRenderer {
  public:
    SdlBlockRenderer(SDL_Renderer *r, RenderConfig cfg) : m_renderer(r), m_config(cfg) {}

    void onResizePixels(int /*pixelW*/, int /*pixelH*/) override {
        // Nothing needed for block renderer; keep for symmetry / future.
    }

    void render(const matrix_rain::MatrixRain &rain) override;

    void present() override { SDL_RenderPresent(m_renderer); }

  private:
    SDL_Renderer *m_renderer = nullptr;
    RenderConfig m_config;
};
