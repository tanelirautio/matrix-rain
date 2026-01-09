#include "rect_packer.hpp"

namespace matrix_rain {

    bool ShelfPacker::startNewShelf() {
        // Move cursor down by the current shelf height.
        m_cursorX = 0;
        m_cursorY += m_shelfH;
        m_shelfH = 0;

        return m_cursorY < m_atlasH;
    }

    std::optional<PackedRect> ShelfPacker::tryPack(int w, int h) {
        // Reject non-positive sizes.
        if (w <= 0 || h <= 0) {
            return std::nullopt;
        }

        // Reject invalid atlas.
        if (m_atlasW <= 0 || m_atlasH <= 0) {
            return std::nullopt;
        }

        // Reserved size includes padding to the right/bottom.
        const int reservedW = w + m_padding;
        const int reservedH = h + m_padding;

        // If the rectangle can never fit, fail early.
        if (reservedW > m_atlasW || reservedH > m_atlasH) {
            return std::nullopt;
        }

        // Ensure we have a shelf; m_shelfH==0 means empty shelf so far.
        // (Nothing special needed; it will get set on first placement.)
        for (int attempt = 0; attempt < 2; ++attempt) {
            // Check if it fits on the current shelf horizontally.
            const bool fitsX = (m_cursorX + reservedW) <= m_atlasW;
            const bool fitsY = (m_cursorY + reservedH) <= m_atlasH;

            if (fitsX && fitsY) {
                PackedRect out;
                out.x = m_cursorX;
                out.y = m_cursorY;
                out.w = w;
                out.h = h;

                // Advance cursor and update shelf height.
                m_cursorX += reservedW;
                m_shelfH = std::max(m_shelfH, reservedH);

                return out;
            }

            // If it doesn't fit horizontally, try starting a new shelf and retry once.
            if (!fitsX) {
                if (!startNewShelf()) {
                    return std::nullopt;
                }

                continue;
            }

            // If it fits X but not Y, atlas is full vertically.
            return std::nullopt;
        }

        return std::nullopt;
    }

} // namespace matrix_rain