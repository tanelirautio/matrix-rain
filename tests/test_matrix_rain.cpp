#include <catch2/catch_test_macros.hpp>
#include <optional>

#include "glyph_set.hpp"
#include "matrix_rain.hpp"

static bool contains(const std::vector<char32_t>& v, char32_t c) {
    for (auto x : v) {
        if (x == c) {
            return true;
        }
    }
    return false;
}

TEST_CASE("MatrixRain constructs with valid grid") {
    matrix_rain::Config cfg{};
    cfg.columns = 10;
    cfg.rows = 5;
    cfg.spawnRatePerColumn = 0.0f;
    cfg.minSpeedRowsPerSecond = 1.0f;
    cfg.maxSpeedRowsPerSecond = 1.0f;
    cfg.fadeRatePerSecond = 1.0f;

    matrix_rain::MatrixRain rain(cfg);

    REQUIRE(rain.columns() == 10);
    REQUIRE(rain.rows() == 5);
}

TEST_CASE("Glyph set contains key characters") {
    auto glyphs = matrix_rain::makeMatrixGlyphSet();

    REQUIRE(contains(glyphs, U'0'));
    REQUIRE(contains(glyphs, U'9'));
    REQUIRE(contains(glyphs, U'A'));
    REQUIRE(contains(glyphs, U'Z'));

    REQUIRE(contains(glyphs, (char32_t)0x30A0)); // Katakana block start
    REQUIRE(contains(glyphs, (char32_t)0x30FF)); // Katakana block end
}

TEST_CASE("Matrix glyph set contains expected ranges") {
    auto glyphs = matrix_rain::makeMatrixGlyphSet();

    REQUIRE(contains(glyphs, U'0'));
    REQUIRE(contains(glyphs, U'9'));
    REQUIRE(contains(glyphs, U'A'));
    REQUIRE(contains(glyphs, U'Z'));

    REQUIRE(contains(glyphs, (char32_t)0x30A0));
    REQUIRE(contains(glyphs, (char32_t)0x30FF));

    const int katakanaCount = (0x30FF - 0x30A0 + 1);
    const int digitCount = 10;
    const int upperCount = 26;
    const int extraCount = 1; // '?'
    REQUIRE((int)glyphs.size() == katakanaCount + digitCount + upperCount + extraCount);
}

TEST_CASE("With fixed seed and high spawn rate, after one update, more than one cell becomes lit in at least one column") {
    matrix_rain::Config cfg{};
    cfg.columns = 5;
    cfg.rows = 30;
    cfg.spawnRatePerColumn = 1000.0f; // very high spawn rate
    cfg.minSpeedRowsPerSecond = 5.0f;
    cfg.maxSpeedRowsPerSecond = 10.0f;
    cfg.fadeRatePerSecond = 0.0f;
    cfg.useFixedSeed = true;
    cfg.rngSeed = 42;
    cfg.minTrailLength = 5;
    cfg.maxTrailLength = 5;

    matrix_rain::MatrixRain rain(cfg);

    for (int i = 0; i < 20; ++i) {
        rain.update(0.1f);
    }

    bool foundColumnWithMultipleLitCells = false;

    for (uint32_t c = 0; c < rain.columns(); ++c) {
        int litCellCount = 0;
        for (uint32_t r = 0; r < rain.rows(); ++r) {
            auto cellState = rain.cell(c, r);
            if (cellState.brightness > 0.0f) {
                litCellCount++;
            }
        }
        if (litCellCount > 1) {
            foundColumnWithMultipleLitCells = true;
            break;
        }
    }

    REQUIRE(foundColumnWithMultipleLitCells);
}

TEST_CASE("If the head stays in the same cell across multiple updates, glyph shouldn't change") {
    matrix_rain::Config cfg{};
    cfg.columns = 1;
    cfg.rows = 10;
    cfg.spawnRatePerColumn = 1000.0f; // force spawn
    cfg.minSpeedRowsPerSecond = 5.0f; // fast enough to enter screen
    cfg.maxSpeedRowsPerSecond = 5.0f;
    cfg.fadeRatePerSecond = 0.0f;
    cfg.useFixedSeed = true;
    cfg.rngSeed = 123;
    cfg.minTrailLength = 2;
    cfg.maxTrailLength = 2;
    cfg.spawnStartAboveMaxRows = 1;
    cfg.glyphChangeRatePerSecond = 0.0f;

    matrix_rain::MatrixRain rain(cfg);

    auto tryFindHead = [&]() -> std::optional<std::pair<uint32_t, char32_t>> {
        float bestB = 0.0f;
        uint32_t bestR = 0;
        char32_t bestG = U' ';

        for (uint32_t r = 0; r < rain.rows(); ++r) {
            auto cell = rain.cell(0, r);
            if (cell.brightness > bestB) {
                bestB = cell.brightness;
                bestR = r;
                bestG = cell.glyph;
            }
        }

        if (bestB <= 0.0f) {
            return std::nullopt;
        }
        return std::make_pair(bestR, bestG);
    };

    // Warm-up: advance until something is lit.
    bool lit = false;
    for (int i = 0; i < 200; ++i) { // 200 * 0.05s = 10s, plenty
        rain.update(0.05f);
        if (tryFindHead().has_value()) {
            lit = true;
            break;
        }
    }
    REQUIRE(lit);

    // Now do two tiny steps so head almost certainly stays in same cell.
    auto h1 = tryFindHead();
    REQUIRE(h1);

    rain.update(0.001f); // 5 rows/sec * 0.001 = 0.005 rows

    auto h2 = tryFindHead();
    REQUIRE(h2);

    REQUIRE(h1->first == h2->first);   // same row
    REQUIRE(h1->second == h2->second); // same glyph
}
