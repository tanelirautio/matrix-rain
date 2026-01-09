#include <algorithm>
#include <cassert>
#include <optional>

namespace matrix_rain {

    struct PackedRect {
        int x = 0, y = 0, w = 0, h = 0;
    };

    class ShelfPacker {
      public:
        ShelfPacker(int atlasW, int atlasH, int paddingPx = 1)
            : m_atlasW(atlasW), m_atlasH(atlasH), m_padding(std::max(0, paddingPx)), m_cursorX(0), m_cursorY(0), m_shelfH(0) {
            assert(m_atlasW > 0 && m_atlasH > 0);
        }

        // Optional helpers (useful for tests / debugging)
        int atlasWidth() const noexcept { return m_atlasW; }
        int atlasHeight() const noexcept { return m_atlasH; }
        int padding() const noexcept { return m_padding; }

        // Reset to empty atlas.
        void reset() noexcept {
            m_cursorX = 0;
            m_cursorY = 0;
            m_shelfH = 0;
        }

        // Try to place a rectangle of size (w,h) into the atlas.
        std::optional<PackedRect> tryPack(int w, int h);

        bool startNewShelf();

      private:
        // Atlas dimensions
        int m_atlasW = 0;
        int m_atlasH = 0;

        // Extra empty pixels reserved to the right/bottom of each placed rect.
        int m_padding = 0;

        // Current packing cursor (top-left of next placement) within the current shelf.
        int m_cursorX = 0;
        int m_cursorY = 0;

        // Current shelf height in pixels (max reserved height of rects placed on this shelf).
        int m_shelfH = 0;
    };

} // namespace matrix_rain
