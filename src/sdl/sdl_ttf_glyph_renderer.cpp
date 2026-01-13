#include "sdl_ttf_glyph_renderer.hpp"

#include "glyph_set.hpp"
#include "matrix_rain.hpp"
#include "sdl_ttf_rasterizer.hpp"

#include "debug_image_io.hpp"

#include <algorithm> // clamp
#include <iostream>
#include <stdexcept>

SdlTtfGlyphRenderer::SdlTtfGlyphRenderer(SDL_Renderer* r, RenderConfig cfg, std::string fontPath, float fontPtSize, bool debugDumps)
    : m_renderer(r), m_config(cfg), m_fontPath(std::move(fontPath)), m_fontPtSize(fontPtSize), m_debugDumps(debugDumps) {
    m_font = TTF_OpenFont(m_fontPath.c_str(), m_fontPtSize);
    if (!m_font) {
        throw std::runtime_error("TTF_OpenFont failed: " + std::string(SDL_GetError()));
    }
    if (!buildAtlas1024()) {
        throw std::runtime_error("Failed to build glyph atlas (try smaller cell size).");
    }
}

SdlTtfGlyphRenderer::~SdlTtfGlyphRenderer() {
    if (m_atlasTex) {
        SDL_DestroyTexture(m_atlasTex);
        m_atlasTex = nullptr;
    }

    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
}

void SdlTtfGlyphRenderer::render(const matrix_rain::MatrixRain& rain) {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    if (!m_atlas || !m_atlasTex) {
        return;
    }

    for (uint32_t row = 0; row < rain.rows(); ++row) {
        for (uint32_t col = 0; col < rain.columns(); ++col) {
            const auto cell = rain.cell(col, row);
            if (cell.brightness <= 0.0f) {
                continue;
            }

            auto rectIt = m_atlas->rects.find(cell.glyph);
            if (rectIt == m_atlas->rects.end()) {
                rectIt = m_atlas->rects.find(U'?');
                if (rectIt == m_atlas->rects.end()) {
                    continue;
                }
            }

            const matrix_rain::PackedRect& r = rectIt->second;

            SDL_FRect src;
            src.x = static_cast<float>(r.x);
            src.y = static_cast<float>(r.y);
            src.w = static_cast<float>(r.w);
            src.h = static_cast<float>(r.h);

            SDL_FRect dst;
            dst.x = static_cast<float>(col * m_config.cellWidthPx);
            dst.y = static_cast<float>(row * m_config.cellHeightPx);
            dst.w = static_cast<float>(m_config.cellWidthPx);
            dst.h = static_cast<float>(m_config.cellHeightPx);

            const float b = std::clamp(cell.brightness, 0.0f, 1.0f);

            if (b > 0.95f) {
                SDL_SetTextureColorMod(m_atlasTex, 180, 255, 180); // head
            } else {
                const Uint8 gval = static_cast<Uint8>(b * 255.0f);
                SDL_SetTextureColorMod(m_atlasTex, 0, gval, 0); // trail
            }

            SDL_SetTextureAlphaMod(m_atlasTex, 255);

            matrix_rain_render_texture(m_renderer, m_atlasTex, &src, &dst);
        }
    }
}

bool SdlTtfGlyphRenderer::buildAtlas1024() {
    // destroy previous atlas texture if rebuilding
    if (m_atlasTex) {
        SDL_DestroyTexture(m_atlasTex);
        m_atlasTex = nullptr;
    }
    m_atlas.reset();

    if (!m_font || !m_renderer) {
        return false;
    }

    const int atlasW = 1024;
    const int atlasH = 1024;
    const int padding = 1;

    matrix_rain::GlyphAtlasBuilder builder(atlasW, atlasH, padding);

    // Rasterizer produces A8 bitmaps (alpha mask).
    SdlTtfRasterizer rast(m_font, m_config.cellWidthPx, m_config.cellHeightPx);

    const auto glyphs = matrix_rain::makeMatrixGlyphSet();

    int added = 0;
    for (char32_t cp : glyphs) {
        auto bm = rast.rasterize(cp);
        if (!bm) {
            continue; // font might not contain this glyph
        }
        if (builder.add(*bm)) {
            ++added;
        }
    }

    if (added == 0) {
        std::cerr << "Atlas build: no glyphs rasterized.\n";
        return false;
    }

    auto atlasOpt = builder.build();
    if (!atlasOpt) {
        if (m_debugDumps) {
            std::cerr << "Atlas build: packing failed (try bigger atlas or fewer glyphs).\n";
        }
        return false;
    }

    // Keep atlas CPU-side for rect lookup and optional debugging
    m_atlas = std::move(*atlasOpt);

    // Ensure fallback exists
    if (m_atlas->rects.find(U'?') == m_atlas->rects.end()) {
        std::cerr << "Atlas build: missing '?' fallback glyph.\n";
        // You can return false OR keep going and skip missing glyphs.
        return false;
    }

    if (m_debugDumps) {
        matrix_rain::writePGM("atlas_debug.pgm", m_atlas->atlasW, m_atlas->atlasH, m_atlas->pixels);
    }

    // Upload to GPU: expand A8 -> RGBA (white w/ alpha)
    const int W = m_atlas->atlasW;
    const int H = m_atlas->atlasH;

    std::vector<std::uint8_t> rgba;
    rgba.resize(static_cast<size_t>(W) * static_cast<size_t>(H) * 4);

    for (int i = 0; i < W * H; ++i) {
        const std::uint8_t a = m_atlas->pixels[static_cast<size_t>(i)];
        rgba[static_cast<size_t>(i) * 4 + 0] = 255;
        rgba[static_cast<size_t>(i) * 4 + 1] = 255;
        rgba[static_cast<size_t>(i) * 4 + 2] = 255;
        rgba[static_cast<size_t>(i) * 4 + 3] = a;
    }

#if MATRIX_RAIN_SDL2
    SDL_Surface* atlasSurface = SDL_CreateRGBSurfaceWithFormatFrom(
        rgba.data(), W, H, 32, W * 4, matrix_rain_rgba_pixel_format());
    if (!atlasSurface) {
        std::cerr << "SDL_CreateRGBSurfaceWithFormatFrom failed: " << SDL_GetError() << "\n";
        m_atlas.reset();
        return false;
    }
    m_atlasTex = SDL_CreateTextureFromSurface(m_renderer, atlasSurface);
    matrix_rain_destroy_surface(atlasSurface);
    if (!m_atlasTex) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
        m_atlas.reset();
        return false;
    }
#else
    m_atlasTex = matrix_rain_create_texture(m_renderer, matrix_rain_rgba_pixel_format(), SDL_TEXTUREACCESS_STATIC, W, H);
    if (!m_atlasTex) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << "\n";
        m_atlas.reset();
        return false;
    }

    if (!matrix_rain_update_texture(m_atlasTex, nullptr, rgba.data(), W * 4)) {
        std::cerr << "SDL_UpdateTexture failed: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(m_atlasTex);
        m_atlasTex = nullptr;
        m_atlas.reset();
        return false;
    }
#endif

    if (SDL_SetTextureBlendMode(m_atlasTex, SDL_BLENDMODE_BLEND) != 0) {
#ifdef __EMSCRIPTEN__
        std::fprintf(stderr, "MatrixRain web: SDL_SetTextureBlendMode failed: %s\n", SDL_GetError());
        std::fflush(stderr);
#else
        std::cerr << "SDL_SetTextureBlendMode failed: " << SDL_GetError() << "\n";
#endif
    }
    return true;
}
