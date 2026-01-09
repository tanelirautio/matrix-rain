#include "sdl_block_renderer.hpp"

#include "matrix_rain.hpp"

#include <algorithm> // std::clamp

void SdlBlockRenderer::render(const matrix_rain::MatrixRain& rain) {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // Draw matrix rain as green blocks
    for (uint32_t row = 0; row < rain.rows(); ++row) {
        for (uint32_t col = 0; col < rain.columns(); ++col) {
            auto cell = rain.cell(col, row);
            if (cell.brightness <= 0.0f) {
                continue;
            }

            float b = std::clamp(cell.brightness, 0.0f, 1.0f);
            Uint8 g = static_cast<Uint8>(b * 255.0f);

            if (b > 0.95f) {
                SDL_SetRenderDrawColor(m_renderer, 180, 255, 180, 255); // pale green / “white-ish”
            } else {
                SDL_SetRenderDrawColor(m_renderer, 0, g, 0, 255);
            }

            SDL_FRect rect;
            rect.x = static_cast<float>(col * m_config.cellWidthPx);
            rect.y = static_cast<float>(row * m_config.cellHeightPx);
            rect.w = static_cast<float>(m_config.cellWidthPx);
            rect.h = static_cast<float>(m_config.cellHeightPx);

            SDL_RenderFillRect(m_renderer, &rect);
        }
    }
}
