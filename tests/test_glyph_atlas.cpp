#include <catch2/catch_test_macros.hpp>

#include "glyph_atlas.hpp"

#include <cstdint>
#include <optional>

TEST_CASE("GlyphAtlasBuilder: build returns nullopt when no glyphs added") {
    using namespace matrix_rain;

    GlyphAtlasBuilder b(32, 32, 1);
    auto atlas = b.build();
    REQUIRE_FALSE(atlas.has_value());
}

TEST_CASE("GlyphAtlasBuilder: add rejects invalid dimensions") {
    using namespace matrix_rain;

    GlyphAtlasBuilder b(32, 32, 1);

    GlyphBitmap g;
    g.glyph = U'A';
    g.w = 0;
    g.h = 10;
    g.fmt = PixelFormat::A8;
    g.pixels = {};

    REQUIRE_FALSE(b.add(g));
}

TEST_CASE("GlyphAtlasBuilder: add rejects null glyph") {
    using namespace matrix_rain;

    GlyphAtlasBuilder b(32, 32, 1);

    GlyphBitmap g;
    g.glyph = U'\0';
    g.w = 2;
    g.h = 2;
    g.fmt = PixelFormat::A8;
    g.pixels = {1, 2, 3, 4};

    REQUIRE_FALSE(b.add(g));
}

TEST_CASE("GlyphAtlasBuilder: add rejects wrong pixel count") {
    using namespace matrix_rain;

    GlyphAtlasBuilder b(32, 32, 1);

    GlyphBitmap g;
    g.glyph = U'A';
    g.w = 3;
    g.h = 2;
    g.fmt = PixelFormat::A8;
    g.pixels = {10, 20, 30, 40, 50}; // should be 6

    REQUIRE_FALSE(b.add(g));
}

TEST_CASE("GlyphAtlasBuilder: add rejects duplicate glyph") {
    using namespace matrix_rain;

    GlyphAtlasBuilder b(32, 32, 1);

    GlyphBitmap g;
    g.glyph = U'A';
    g.w = 2;
    g.h = 2;
    g.fmt = PixelFormat::A8;
    g.pixels = {1, 2, 3, 4};

    REQUIRE(b.add(g));
    REQUIRE_FALSE(b.add(g));
}

TEST_CASE("GlyphAtlasBuilder: build creates atlas, rect entry, and blits pixels") {
    using namespace matrix_rain;

    GlyphAtlasBuilder b(32, 32, 1);

    GlyphBitmap g;
    g.glyph = U'A';
    g.w = 3;
    g.h = 2;
    g.fmt = PixelFormat::A8;

    // 3x2 pattern:
    // 10 20 30
    // 40 50 60
    g.pixels = {10, 20, 30, 40, 50, 60};

    REQUIRE(b.add(g));

    auto atlasOpt = b.build();
    REQUIRE(atlasOpt);

    const GlyphAtlas& atlas = *atlasOpt;
    REQUIRE(atlas.atlasW == 32);
    REQUIRE(atlas.atlasH == 32);
    REQUIRE(atlas.padding == 1);
    REQUIRE(atlas.fmt == PixelFormat::A8);

    // must have backing pixel buffer
    REQUIRE(atlas.pixels.size() == static_cast<std::size_t>(atlas.atlasW) * static_cast<std::size_t>(atlas.atlasH));

    // must have rect for glyph
    auto it = atlas.rects.find(U'A');
    REQUIRE(it != atlas.rects.end());

    const PackedRect& r = it->second;
    REQUIRE(r.w == 3);
    REQUIRE(r.h == 2);

    // helper to read from atlas at (x,y)
    auto at = [&](int x, int y) -> std::uint8_t {
        REQUIRE(x >= 0);
        REQUIRE(y >= 0);
        REQUIRE(x < atlas.atlasW);
        REQUIRE(y < atlas.atlasH);
        return atlas.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(atlas.atlasW) + static_cast<std::size_t>(x)];
    };

    // verify our 3x2 pattern landed at r.x,r.y
    REQUIRE(at(r.x + 0, r.y + 0) == 10);
    REQUIRE(at(r.x + 1, r.y + 0) == 20);
    REQUIRE(at(r.x + 2, r.y + 0) == 30);
    REQUIRE(at(r.x + 0, r.y + 1) == 40);
    REQUIRE(at(r.x + 1, r.y + 1) == 50);
    REQUIRE(at(r.x + 2, r.y + 1) == 60);
}

TEST_CASE("GlyphAtlasBuilder: build returns nullopt if a glyph does not fit") {
    using namespace matrix_rain;

    // Atlas 8x8 can't fit a 9x1 glyph (regardless of padding)
    GlyphAtlasBuilder b(8, 8, 1);

    GlyphBitmap g;
    g.glyph = U'X';
    g.w = 9;
    g.h = 1;
    g.fmt = PixelFormat::A8;
    g.pixels.assign(static_cast<std::size_t>(g.w) * static_cast<std::size_t>(g.h), 255);

    REQUIRE(b.add(g));

    auto atlasOpt = b.build();
    REQUIRE_FALSE(atlasOpt.has_value());
}

TEST_CASE("GlyphAtlasBuilder: multiple glyphs do not overlap and both patterns are blitted") {
    matrix_rain::GlyphAtlasBuilder b(64, 64, 1);

    // Glyph A: 3x2
    matrix_rain::GlyphBitmap a;
    a.glyph = U'A';
    a.w = 3;
    a.h = 2;
    a.fmt = matrix_rain::PixelFormat::A8;
    a.pixels = {10, 20, 30, 40, 50, 60};

    // Glyph B: 2x3 (different shape + distinct values)
    matrix_rain::GlyphBitmap bb;
    bb.glyph = U'B';
    bb.w = 2;
    bb.h = 3;
    bb.fmt = matrix_rain::PixelFormat::A8;
    bb.pixels = {1, 2, 3, 4, 5, 6};

    REQUIRE(b.add(a));
    REQUIRE(b.add(bb));

    auto atlasOpt = b.build();
    REQUIRE(atlasOpt);

    const matrix_rain::GlyphAtlas& atlas = *atlasOpt;

    // Find rects
    auto itA = atlas.rects.find(U'A');
    auto itB = atlas.rects.find(U'B');
    REQUIRE(itA != atlas.rects.end());
    REQUIRE(itB != atlas.rects.end());

    const matrix_rain::PackedRect& rA = itA->second;
    const matrix_rain::PackedRect& rB = itB->second;

    // Basic sanity
    REQUIRE(rA.w == a.w);
    REQUIRE(rA.h == a.h);
    REQUIRE(rB.w == bb.w);
    REQUIRE(rB.h == bb.h);

    // Must not overlap (strict AABB overlap test)
    const bool overlapX = (rA.x < rB.x + rB.w) && (rB.x < rA.x + rA.w);
    const bool overlapY = (rA.y < rB.y + rB.h) && (rB.y < rA.y + rA.h);
    REQUIRE_FALSE((overlapX && overlapY));

    // Helper: read atlas pixel
    auto at = [&](int x, int y) -> std::uint8_t {
        REQUIRE(x >= 0);
        REQUIRE(y >= 0);
        REQUIRE(x < atlas.atlasW);
        REQUIRE(y < atlas.atlasH);
        return atlas.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(atlas.atlasW) + static_cast<std::size_t>(x)];
    };

    // Verify A pattern landed at rA
    REQUIRE(at(rA.x + 0, rA.y + 0) == 10);
    REQUIRE(at(rA.x + 1, rA.y + 0) == 20);
    REQUIRE(at(rA.x + 2, rA.y + 0) == 30);
    REQUIRE(at(rA.x + 0, rA.y + 1) == 40);
    REQUIRE(at(rA.x + 1, rA.y + 1) == 50);
    REQUIRE(at(rA.x + 2, rA.y + 1) == 60);

    // Verify B pattern landed at rB
    REQUIRE(at(rB.x + 0, rB.y + 0) == 1);
    REQUIRE(at(rB.x + 1, rB.y + 0) == 2);
    REQUIRE(at(rB.x + 0, rB.y + 1) == 3);
    REQUIRE(at(rB.x + 1, rB.y + 1) == 4);
    REQUIRE(at(rB.x + 0, rB.y + 2) == 5);
    REQUIRE(at(rB.x + 1, rB.y + 2) == 6);
}
