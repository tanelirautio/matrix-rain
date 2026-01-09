#include "glyph_atlas.hpp"

#include <algorithm> // std::fill
#include <cstddef>   // std::size_t

namespace matrix_rain {

    GlyphAtlasBuilder::GlyphAtlasBuilder(int atlasW, int atlasH, int padding) : m_atlasW(atlasW), m_atlasH(atlasH), m_padding(padding) {}

    bool GlyphAtlasBuilder::add(const GlyphBitmap& glyph) {
        // Validate
        if (glyph.w <= 0 || glyph.h <= 0) {
            return false;
        }
        if (glyph.fmt != PixelFormat::A8) {
            return false; // keep simple for now
        }
        if (glyph.glyph == U'\0') {
            return false;
        }
        const std::size_t expected = static_cast<std::size_t>(glyph.w) * static_cast<std::size_t>(glyph.h);
        if (glyph.pixels.size() != expected) {
            return false;
        }

        // Reject duplicates
        for (const auto& g : m_glyphs) {
            if (g.glyph == glyph.glyph) {
                return false;
            }
        }

        m_glyphs.push_back(glyph);
        return true;
    }

    static void blitA8(std::uint8_t* dst, int dstW, int dstX, int dstY, const std::uint8_t* src, int srcW, int srcH) {
        // Assume inputs are valid and in-bounds (caller guarantees packing).
        for (int y = 0; y < srcH; ++y) {
            const int dy = dstY + y;
            std::uint8_t* dstRow = dst + dy * dstW + dstX;
            const std::uint8_t* srcRow = src + y * srcW;
            std::copy(srcRow, srcRow + srcW, dstRow);
        }
    }

    std::optional<GlyphAtlas> GlyphAtlasBuilder::build() {
        GlyphAtlas atlas{};
        atlas.atlasW = m_atlasW;
        atlas.atlasH = m_atlasH;
        atlas.padding = m_padding;
        atlas.fmt = PixelFormat::A8;

        if (m_atlasW <= 0 || m_atlasH <= 0 || m_glyphs.empty()) {
            return std::nullopt;
        }

        ShelfPacker packer(m_atlasW, m_atlasH, m_padding);

        // Pack all glyphs first (no pixel writes yet)
        for (const auto& g : m_glyphs) {
            auto r = packer.tryPack(g.w, g.h);
            if (!r) {
                return std::nullopt;
            }
            atlas.rects[g.glyph] = *r;
        }

        // Allocate atlas pixels and clear
        atlas.pixels.assign(static_cast<std::size_t>(m_atlasW) * static_cast<std::size_t>(m_atlasH), std::uint8_t{0});

        // Blit glyph pixels into atlas
        for (const auto& g : m_glyphs) {
            const auto it = atlas.rects.find(g.glyph);
            if (it == atlas.rects.end()) {
                return std::nullopt; // should never happen
            }
            const PackedRect& r = it->second;

            blitA8(atlas.pixels.data(), atlas.atlasW, r.x, r.y, g.pixels.data(), g.w, g.h);
        }

        return atlas;
    }

} // namespace matrix_rain
