#pragma once

#include <cstdint>

namespace matrix_rain {
class MatrixRain;
}

struct RenderConfig {
    int cellWidthPx = 16;
    int cellHeightPx = 16;
};

class IRenderer {
  public:
    virtual ~IRenderer() = default;

    virtual void onResizePixels(int pixelW, int pixelH) = 0;

    // Render one frame of the current simulation state
    virtual void render(const matrix_rain::MatrixRain &rain) = 0;

    // Optional: for backends that need presenting
    virtual void present() = 0;
};
