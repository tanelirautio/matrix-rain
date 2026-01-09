#include "matrix_rain.hpp"
#include <algorithm>
#include <cmath>
#include <random>

#include "glyph_set.hpp"

namespace matrix_rain {
    namespace {
        float clamp_dt(float dt) {
            // Prevent huge dt after a breakpoint/hitch from making everything jump.
            // 0.1s means we simulate at worst like 10 FPS.
            return std::clamp(dt, 0.0f, 0.1f);
        }

        float rand01(std::mt19937& rng) {
            static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            return dist(rng);
        }

        float rand_range(std::mt19937& rng, float a, float b) {
            if (a > b) {
                std::swap(a, b);
            }
            std::uniform_real_distribution<float> dist(a, b);
            return dist(rng);
        }

        uint32_t rand_uint(std::mt19937& rng, uint32_t a, uint32_t b) {
            if (a > b) {
                std::swap(a, b);
            }
            std::uniform_int_distribution<uint32_t> dist(a, b);
            return dist(rng);
        }

        int32_t rand_int(std::mt19937& rng, int32_t a, int32_t b) {
            if (a > b) {
                std::swap(a, b);
            }
            std::uniform_int_distribution<int32_t> dist(a, b);
            return dist(rng);
        }

        char32_t random_glyph(std::mt19937& rng) {
            static const std::vector<char32_t> glyphs = matrix_rain::makeMatrixGlyphSet();
            const uint32_t n = static_cast<uint32_t>(glyphs.size());
            return glyphs[rand_uint(rng, 0, n - 1)];
        }

    } // namespace

    matrix_rain::MatrixRain::MatrixRain(const Config& cfg)
        : m_config(cfg), m_drops(cfg.columns), m_glyphs(cfg.columns * cfg.rows, U' '), m_brightness(cfg.columns * cfg.rows, 0.0f) {
        // Seed RNG
        if (cfg.useFixedSeed) {
            m_rng = std::mt19937(cfg.rngSeed);
        } else {
            std::random_device rd;
            m_rng = std::mt19937(rd());
        }

        // Init drop slots (one per column)
        for (uint32_t c = 0; c < m_config.columns; ++c) {
            m_drops[c].active = false;
            m_drops[c].column = c;
            m_drops[c].headRow = 0.0f;
            m_drops[c].speedRowsPerSec = 0.0f;
        }
    }

    void MatrixRain::update(float deltaSeconds) {
        deltaSeconds = clamp_dt(deltaSeconds);

        const uint32_t cols = m_config.columns;
        const uint32_t rows = m_config.rows;
        if (cols == 0 || rows == 0) {
            return;
        }

        // --- 1) Fade all cells (exponential decay) ---
        //
        // brightness(t + dt) = brightness(t) * exp(-k * dt)
        // k = fadeRatePerSecond
        const float k = std::max(0.0f, m_config.fadeRatePerSecond);
        const float fadeFactor = std::exp(-k * deltaSeconds);

        for (float& b : m_brightness) {
            b *= fadeFactor;
            if (b < 0.002f) {
                b = 0.0f; // tiny cutoff to keep things crisp
            }
        }

        // --- 2) Glyph “shimmer” in lit cells (optional but very Matrix-y) ---
        //
        // A little random glyph mutation makes the rain feel alive.
        // Keep the probability small.
        const float glyphChangeP = 1.0f - std::exp(-m_config.glyphChangeRatePerSecond * deltaSeconds);

        for (uint32_t i = 0; i < m_brightness.size(); ++i) {
            if (m_brightness[i] <= 0.0f) {
                continue;
            }
            if (rand01(m_rng) < glyphChangeP) {
                m_glyphs[i] = random_glyph(m_rng);
            }
        }

        // --- 3) Spawn probability per column for this dt ---
        const float rate = std::max(0.0f, m_config.spawnRatePerColumn);
        const float lambda = rate * deltaSeconds;
        const float spawnP = 1.0f - std::exp(-lambda);

        // --- 4) Update drops (move + draw head), and spawn new ones ---
        for (uint32_t c = 0; c < cols; ++c) {
            Drop& d = m_drops[c];

            // Spawn if inactive
            if (!d.active) {
                if (rand01(m_rng) < spawnP) {
                    d.active = true;
                    d.column = c;

                    d.speedRowsPerSec = rand_range(m_rng, m_config.minSpeedRowsPerSecond, m_config.maxSpeedRowsPerSecond);

                    const uint32_t defaultMaxAbove = rows / 2; // allow 0
                    const uint32_t maxAbove = (m_config.spawnStartAboveMaxRows > 0) ? m_config.spawnStartAboveMaxRows : defaultMaxAbove;
                    const float startAbove = static_cast<float>(rand_uint(m_rng, 0, maxAbove));
                    d.headRow = -startAbove;

                    d.trailLength = std::max(1, rand_int(m_rng, m_config.minTrailLength, m_config.maxTrailLength));
                    d.lastHeadCell = static_cast<int>(std::floor(d.headRow)) - 1;
                }

                continue;
            }

            // Move head
            d.headRow += d.speedRowsPerSec * deltaSeconds;

            // Draw head if inside screen
            const int head = static_cast<int>(std::floor(d.headRow));
            if (head != d.lastHeadCell) {
                d.lastHeadCell = head;
                if (head >= 0 && head < static_cast<int>(rows)) {
                    const uint32_t idx = static_cast<uint32_t>(head) * cols + c;
                    m_brightness[idx] = 1.0f;
                    m_glyphs[idx] = random_glyph(m_rng); // only on entering new row
                }
            } else {
                // same cell as previous frame: still keep it bright if desired
                if (head >= 0 && head < static_cast<int>(rows)) {
                    const uint32_t idx = static_cast<uint32_t>(head) * cols + c;
                    m_brightness[idx] = std::max(m_brightness[idx], 1.0f);
                }
            }

            // Draw trail
            // Optional in the future - exponential falloff: float trailBrightness = std::exp(-t * 0.35f);

            float invLen = 1.0f / (float)(d.trailLength + 1);
            for (int t = 1; t <= d.trailLength; ++t) {
                int row = head - t;
                if (row >= 0 && row < static_cast<int>(rows)) {
                    const uint32_t idx = static_cast<uint32_t>(row) * cols + c;
                    // Trail brightness falls off linearly from head to tail
                    float trailBrightness = 1.0f - t * invLen;
                    // Only increase brightness if it's higher than current (to avoid overwriting brighter parts)
                    m_brightness[idx] = std::max(m_brightness[idx], trailBrightness);
                    // Optionally, could also randomize glyphs in the trail, but keeping them stable looks better
                }
            }

            // Deactivate after it has moved well below screen.
            // Since trail is implicit (fade), give it some margin so it doesn't
            // disappear too early.
            if (d.headRow > static_cast<float>(rows) + 10.0f) {
                d.active = false;
            }
        }
    }

    std::uint32_t MatrixRain::columns() const noexcept {
        return m_config.columns;
    }

    std::uint32_t MatrixRain::rows() const noexcept {
        return m_config.rows;
    }

    CellState MatrixRain::cell(std::uint32_t column, std::uint32_t row) const noexcept {
        const uint32_t idx = row * m_config.columns + column;
        return CellState{m_glyphs[idx], m_brightness[idx]};
    }

} // namespace matrix_rain