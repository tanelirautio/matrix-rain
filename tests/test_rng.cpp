#include <catch2/catch_test_macros.hpp>

#include "glyph_set.hpp"
#include "matrix_rain.hpp"
#include "rect_packer.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

// Helpers

static std::vector<std::uint64_t> snapshotGrid(const matrix_rain::MatrixRain& rain) {
    std::vector<std::uint64_t> out;
    out.reserve(static_cast<size_t>(rain.columns()) * rain.rows());

    for (uint32_t row = 0; row < rain.rows(); ++row) {
        for (uint32_t col = 0; col < rain.columns(); ++col) {
            auto cell = rain.cell(col, row);

            // Quantize brightness so tiny float differences don't cause noise.
            // (Your sim should be deterministic anyway, but this keeps tests robust.)
            uint32_t bq = static_cast<uint32_t>(cell.brightness * 100000.0f + 0.5f);

            std::uint64_t packed = (static_cast<std::uint64_t>(cell.glyph) << 32) | static_cast<std::uint64_t>(bq);
            out.push_back(packed);
        }
    }

    return out;
}

static matrix_rain::Config makeTestConfig() {
    matrix_rain::Config cfg{};
    cfg.columns = 8;
    cfg.rows = 6;

    // Make spawns likely and visible quickly.
    cfg.spawnRatePerColumn = 50.0f;

    cfg.minSpeedRowsPerSecond = 10.0f;
    cfg.maxSpeedRowsPerSecond = 10.0f;

    // Keep fading slow so brightness persists for comparison.
    cfg.fadeRatePerSecond = 0.2f;

    return cfg;
}

// Test cases

TEST_CASE("Fixed RNG seed makes simulation reproducible") {
    auto cfg = makeTestConfig();
    cfg.useFixedSeed = true;
    cfg.rngSeed = 123u;

    matrix_rain::MatrixRain a(cfg);
    matrix_rain::MatrixRain b(cfg);

    for (int i = 0; i < 20; ++i) {
        a.update(0.1f);
        b.update(0.1f);
    }

    REQUIRE(snapshotGrid(a) == snapshotGrid(b));
}

TEST_CASE("Different RNG seeds produce different outcomes") {
    auto cfgA = makeTestConfig();
    cfgA.useFixedSeed = true;
    cfgA.rngSeed = 1u;

    auto cfgB = makeTestConfig();
    cfgB.useFixedSeed = true;
    cfgB.rngSeed = 2u;

    matrix_rain::MatrixRain a(cfgA);
    matrix_rain::MatrixRain b(cfgB);

    for (int i = 0; i < 20; ++i) {
        a.update(0.1f);
        b.update(0.1f);
    }

    REQUIRE(snapshotGrid(a) != snapshotGrid(b));
}

TEST_CASE("Brightness stays within [0,1] and is finite") {
    auto cfg = makeTestConfig();
    cfg.rngSeed = 777u;

    matrix_rain::MatrixRain rain(cfg);

    // Run for a while to exercise spawn, shimmer, fade.
    for (int i = 0; i < 200; ++i) {
        rain.update(0.1f);
    }

    for (uint32_t row = 0; row < rain.rows(); ++row) {
        for (uint32_t col = 0; col < rain.columns(); ++col) {
            auto cell = rain.cell(col, row);
            REQUIRE(std::isfinite(cell.brightness));
            REQUIRE(cell.brightness >= 0.0f);
            REQUIRE(cell.brightness <= 1.0f);
        }
    }
}
