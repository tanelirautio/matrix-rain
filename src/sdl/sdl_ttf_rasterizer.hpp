#pragma once

#include <cstdint>
#include <optional>

#include "sdl_compat.hpp"

#include "glyph_atlas.hpp" // matrix_rain::GlyphBitmap / PixelFormat

class SdlTtfRasterizer {
  public:
    SdlTtfRasterizer(TTF_Font* font, int cellW, int cellH);

    // Returns A8 alpha mask in a fixed-size cell (cellW x cellH).
    // nullopt if glyph can't be rendered.
    std::optional<matrix_rain::GlyphBitmap> rasterize(char32_t cp) const;

  private:
    TTF_Font* m_font = nullptr;
    int m_cellW = 0;
    int m_cellH = 0;
};
