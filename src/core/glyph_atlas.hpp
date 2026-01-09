#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "rect_packer.hpp"

namespace matrix_rain {

    enum class PixelFormat {
        A8, // 1 byte per pixel (alpha mask)
    };

    struct GlyphBitmap {
        char32_t glyph = U'?';
        int w = 0;
        int h = 0;
        PixelFormat fmt = PixelFormat::A8;

        // For A8: size = w*h
        std::vector<std::uint8_t> pixels;
    };

    struct GlyphAtlas {
        int atlasW = 0;
        int atlasH = 0;
        int padding = 0;
        PixelFormat fmt = PixelFormat::A8;

        // For A8: size = atlasW*atlasH
        std::vector<std::uint8_t> pixels;

        // maps glyph -> rectangle in atlas
        std::unordered_map<char32_t, PackedRect> rects;
    };

    class GlyphAtlasBuilder {
      public:
        GlyphAtlasBuilder(int atlasW, int atlasH, int padding);

        // Adds a glyph bitmap. Returns false if already added or invalid.
        bool add(const GlyphBitmap& glyph);

        // Packs + builds the atlas bitmap. Returns nullopt if something doesn't fit.
        std::optional<GlyphAtlas> build();

      private:
        int m_atlasW = 0;
        int m_atlasH = 0;
        int m_padding = 0;

        // Keep glyphs in insertion order so builds are deterministic.
        std::vector<GlyphBitmap> m_glyphs;
    };

} // namespace matrix_rain
