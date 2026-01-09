#pragma once

#include <cstdint>
#include <random>
#include <vector>

namespace matrix_rain {

    struct Config {
        std::uint32_t columns = 0; // character cells
        std::uint32_t rows = 0;

        float spawnRatePerColumn = 0.0f;    // drops / (column * second)
        float minSpeedRowsPerSecond = 0.0f; // rows / second
        float maxSpeedRowsPerSecond = 0.0f; // rows / second
        float fadeRatePerSecond = 0.0f;     // >= 0

        int minTrailLength = 6;
        int maxTrailLength = 20;

        bool useFixedSeed = false;
        uint32_t rngSeed = 0;

        uint32_t spawnStartAboveMaxRows = 0;   // 0 = default (rows/2), otherwise cap spawn above top
        float glyphChangeRatePerSecond = 2.0f; // 0 disables shimmer
    };

    struct CellState {
        char32_t glyph;   // character code for this cell (e.g. Unicode)
        float brightness; // 0..1, where 0 = fully dark, 1 = fully bright
    };

    // Pure simulation of a "Matrix rain" effect over a discrete character grid.
    class MatrixRain {
      public:
        explicit MatrixRain(const Config& cfg);

        void update(float deltaSeconds);

        std::uint32_t columns() const noexcept;
        std::uint32_t rows() const noexcept;

        CellState cell(std::uint32_t column, std::uint32_t row) const noexcept;

      private:
        struct Drop {
            bool active = false;                                // is there a drop in this column right now?
            uint32_t column = 0;                                // which column this drop belongs to
            float headRow = 0.0f;                               // current head position (in rows, can be fractional)
            float speedRowsPerSec = 0.0f;                       // how fast it moves
            int trailLength = 0;                                // length of the trail (in rows) (head not included)
            int lastHeadCell = std::numeric_limits<int>::min(); // last cell index occupied by head
        };

        Config m_config;
        std::vector<Drop> m_drops;       // size = cfg_.columns
        std::vector<char32_t> m_glyphs;  // columns*rows
        std::vector<float> m_brightness; // columns*rows
        std::mt19937 m_rng;
    };

} // namespace matrix_rain
