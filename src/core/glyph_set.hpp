#pragma once

#include <cstdint>
#include <vector>

namespace matrix_rain {
    inline std::vector<char32_t> makeMatrixGlyphSet() {
        std::vector<char32_t> glyphs;

        // Updated reserve: Katakana (96) + Digits (10) + Uppercase (26) + Question Mark (1)
        glyphs.reserve((0x30FF - 0x30A0 + 1) + 10 + 26 + 1);

        // Katakana U+30A0..U+30FF
        for (char32_t c = 0x30A0; c <= 0x30FF; ++c) {
            glyphs.push_back(c);
        }

        // ASCII digits
        for (char32_t c = U'0'; c <= U'9'; ++c) {
            glyphs.push_back(c);
        }

        // ASCII uppercase
        for (char32_t c = U'A'; c <= U'Z'; ++c) {
            glyphs.push_back(c);
        }

        // Add the question mark
        glyphs.push_back(U'?');

        return glyphs;
    }
} // namespace matrix_rain
