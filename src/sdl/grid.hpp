#pragma once

#include <algorithm>
#include <cstdint>
#include <utility>

inline std::pair<std::uint32_t, std::uint32_t> computeGridFromPixels(int pixelW, int pixelH, int cellW, int cellH) {
    const int safeCellW = std::max(1, cellW);
    const int safeCellH = std::max(1, cellH);
    std::uint32_t cols = static_cast<std::uint32_t>(std::max(1, pixelW / safeCellW));
    std::uint32_t rows = static_cast<std::uint32_t>(std::max(1, pixelH / safeCellH));
    return {cols, rows};
}
