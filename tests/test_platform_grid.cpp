#include <catch2/catch_test_macros.hpp>

#include "grid.hpp"

TEST_CASE("Grid computation uses pixel dimensions") {
    auto [cols, rows] = computeGridFromPixels(640, 480, 32, 32);
    REQUIRE(cols == 20);
    REQUIRE(rows == 15);
}

TEST_CASE("Grid computation changes with resize") {
    auto [colsA, rowsA] = computeGridFromPixels(640, 480, 32, 32);
    auto [colsB, rowsB] = computeGridFromPixels(800, 600, 32, 32);
    REQUIRE(colsB > colsA);
    REQUIRE(rowsB > rowsA);
}

TEST_CASE("Grid computation clamps to at least one cell") {
    auto [cols, rows] = computeGridFromPixels(10, 10, 64, 64);
    REQUIRE(cols == 1);
    REQUIRE(rows == 1);
}
