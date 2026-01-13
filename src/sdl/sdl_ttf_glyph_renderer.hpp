#pragma once

#include "glyph_atlas.hpp"
#include "renderer.hpp"
#include "sdl_compat.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

class SdlTtfGlyphRenderer final : public IRenderer {
  public:
    struct GlyphTex {
        SDL_Texture* tex = nullptr;
        int w = 0;
        int h = 0;
    };

    SdlTtfGlyphRenderer(SDL_Renderer* r, RenderConfig cfg, std::string fontPath, float fontPtSize, bool debugDumps);
    ~SdlTtfGlyphRenderer() override;

    void onResizePixels(int /*pixelW*/, int /*pixelH*/) override {}

    void render(const matrix_rain::MatrixRain& rain) override;
    void present() override { SDL_RenderPresent(m_renderer); }

  private:
    bool buildAtlas1024();

    SDL_Renderer* m_renderer = nullptr;
    RenderConfig m_config;

    std::string m_fontPath;
    float m_fontPtSize = 16.0f;
    TTF_Font* m_font = nullptr;
    bool m_debugDumps = false;

    std::optional<matrix_rain::GlyphAtlas> m_atlas;
    SDL_Texture* m_atlasTex = nullptr;
};
