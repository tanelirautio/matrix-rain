#include <catch2/catch_test_macros.hpp>

#include "rect_packer.hpp"

TEST_CASE("ShelfPacker packs first rectangle at origin") {
    matrix_rain::ShelfPacker packer(100, 100, 1);
    auto rectOpt = packer.tryPack(10, 10);
    REQUIRE(rectOpt);
    auto rect = rectOpt.value();
    REQUIRE(rect.x == 0);
    REQUIRE(rect.y == 0);
    REQUIRE(rect.w == 10);
    REQUIRE(rect.h == 10);
}

TEST_CASE("ShelfPacker respects padding") {
    matrix_rain::ShelfPacker packer(100, 100, 2);
    auto r1 = packer.tryPack(10, 10);
    REQUIRE(r1);
    REQUIRE(r1->x == 0);
    REQUIRE(r1->y == 0);
    REQUIRE(r1->w == 10);
    REQUIRE(r1->h == 10);

    auto r2 = packer.tryPack(10, 10);
    REQUIRE(r2);
    REQUIRE(r2->x == 12);
    REQUIRE(r2->y == 0);
    REQUIRE(r2->w == 10);
    REQUIRE(r2->h == 10);
}

TEST_CASE("ShelfPacker moves to next row when width exceeds") {
    matrix_rain::ShelfPacker packer(15, 100, 1);
    auto rect1Opt = packer.tryPack(10, 10);
    REQUIRE(rect1Opt);
    auto rect1 = rect1Opt.value();
    REQUIRE(rect1.x == 0);
    REQUIRE(rect1.y == 0);

    auto rect2Opt = packer.tryPack(10, 10);
    REQUIRE(rect2Opt);
    auto rect2 = rect2Opt.value();
    REQUIRE(rect2.x == 0);                          // New shelf, so x should be 0
    REQUIRE(rect2.y == rect1.h + packer.padding()); // Moved down by height + padding of first rect
}

TEST_CASE("ShelfPacker fails when atlas is full") {
    matrix_rain::ShelfPacker packer(20, 22, 1);

    auto r1 = packer.tryPack(10, 10);
    REQUIRE(r1);

    auto r2 = packer.tryPack(10, 10);
    REQUIRE(r2); // wraps to y=11 and fits

    auto r3 = packer.tryPack(10, 10);
    REQUIRE(!r3); // would need y=22 -> no space
}

TEST_CASE("ShelfPacker never overlaps rects") {
    matrix_rain::ShelfPacker packer(50, 50, 1);
    std::vector<matrix_rain::PackedRect> packedRects;

    const std::vector<std::pair<int, int>> rectSizes = {{10, 10}, {15, 5}, {5, 20}, {20, 10}, {10, 15}};

    for (const auto& size : rectSizes) {
        auto rectOpt = packer.tryPack(size.first, size.second);
        REQUIRE(rectOpt);

        // In-bounds checks
        REQUIRE(rectOpt->x >= 0);
        REQUIRE(rectOpt->y >= 0);
        REQUIRE(rectOpt->x + rectOpt->w <= packer.atlasWidth());
        REQUIRE(rectOpt->y + rectOpt->h <= packer.atlasHeight());

        packedRects.push_back(*rectOpt);
    }

    for (size_t i = 0; i < packedRects.size(); ++i) {
        const auto& a = packedRects[i];
        for (size_t j = i + 1; j < packedRects.size(); ++j) {
            const auto& b = packedRects[j];

            bool overlapX = (a.x < b.x + b.w) && (b.x < a.x + a.w);
            bool overlapY = (a.y < b.y + b.h) && (b.y < a.y + a.h);

            REQUIRE(!(overlapX && overlapY));
        }
    }
}