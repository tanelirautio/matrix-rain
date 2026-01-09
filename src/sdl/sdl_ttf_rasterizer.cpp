#include "sdl_ttf_rasterizer.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

SdlTtfRasterizer::SdlTtfRasterizer(TTF_Font* font, int cellW, int cellH) : m_font(font), m_cellW(cellW), m_cellH(cellH) {}

std::optional<matrix_rain::GlyphBitmap> SdlTtfRasterizer::rasterize(char32_t cp) const {
    if (!m_font || m_cellW <= 0 || m_cellH <= 0) {
        return std::nullopt;
    }
    if (cp == U'\0') {
        return std::nullopt;
    }

    // Render white; we only keep alpha.
    SDL_Color white{255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderGlyph_Blended(m_font, static_cast<Uint32>(cp), white);
    if (!surf) {
        return std::nullopt;
    }

    // Convert to a known format with alpha so we can read pixels consistently.
    SDL_Surface* rgba = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surf);
    surf = nullptr;

    if (!rgba) {
        return std::nullopt;
    }

    const int srcW = rgba->w;
    const int srcH = rgba->h;

    // Decide placement inside the cell.
    // We'll baseline-align using font metrics + glyph metrics.
    //
    // Baseline Y in your cell:
    //   baselineY = cellH - descent
    //
    // Then we want glyph's top-left such that its baseline matches baselineY.
    //
    // SDL_ttf gives: minx/maxx/miny/maxy/advance where miny/maxy are relative to baseline.
    int minx = 0, maxx = 0, miny = 0, maxy = 0, advance = 0;
    if (!TTF_GetGlyphMetrics(m_font, static_cast<Uint32>(cp), &minx, &maxx, &miny, &maxy, &advance)) {
        SDL_DestroySurface(rgba);
        return std::nullopt;
    }

    const int ascent = TTF_GetFontAscent(m_font);
    const int descent = TTF_GetFontDescent(m_font); // typically <= 0 in some libs; SDL_ttf returns positive? depends.
    // To be safe, treat descent as absolute pixels below baseline:
    const int descentAbs = std::abs(descent);

    // baseline Y in cell
    const int baselineY = m_cellH - descentAbs;

    // miny is usually negative (above baseline). glyph top relative to baseline is miny.
    // We need destination Y = baselineY + miny, then clamp to keep glyph visible in the cell.
    const int maxY = std::max(0, m_cellH - srcH);
    int dstY = std::clamp(baselineY + miny, 0, maxY);

    // Center horizontally based on rendered surface width (simple + good enough for monospace cells).
    const int maxX = std::max(0, m_cellW - srcW);
    int dstX = std::clamp((m_cellW - srcW) / 2, 0, maxX);

    // Build fixed-size A8 output
    matrix_rain::GlyphBitmap out{};
    out.glyph = cp;
    out.w = m_cellW;
    out.h = m_cellH;
    out.fmt = matrix_rain::PixelFormat::A8;
    out.pixels.assign(static_cast<std::size_t>(out.w) * static_cast<std::size_t>(out.h), std::uint8_t{0});

    // Read glyph coverage from the rendered surface (alpha when present, otherwise RGB intensity).
    bool locked = false;
    if (SDL_MUSTLOCK(rgba)) {
        if (!SDL_LockSurface(rgba)) {
            SDL_DestroySurface(rgba);
            return std::nullopt;
        }
        locked = true;
    }

    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(rgba->format);
    const SDL_Palette* palette = SDL_GetSurfacePalette(rgba);
    const int bpp = SDL_BYTESPERPIXEL(rgba->format);
    const std::uint8_t* srcPixels = static_cast<const std::uint8_t*>(rgba->pixels);
    const int srcPitch = rgba->pitch;

    auto read_rgba = [&](const std::uint8_t* p, Uint8& r, Uint8& g, Uint8& b, Uint8& a) {
        if (!fmt) {
            r = g = b = a = 0;
            return;
        }
        Uint32 pixel = 0;
        switch (bpp) {
        case 1: {
            pixel = *p;
        } break;
        case 2: {
            Uint16 tmp = 0;
            std::memcpy(&tmp, p, sizeof(tmp));
            pixel = tmp;
        } break;
        case 3: {
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                pixel = (p[0] << 16) | (p[1] << 8) | p[2];
            } else {
                pixel = p[0] | (p[1] << 8) | (p[2] << 16);
            }
        } break;
        case 4: {
            Uint32 tmp = 0;
            std::memcpy(&tmp, p, sizeof(tmp));
            pixel = tmp;
        } break;
        default:
            pixel = 0;
            break;
        }
        SDL_GetRGBA(pixel, fmt, palette, &r, &g, &b, &a);
    };

    Uint8 maxA = 0;
    Uint8 maxRGB = 0;
    std::uint64_t sumRGB = 0;
    const int pixelCount = srcW * srcH;

    for (int y = 0; y < srcH; ++y) {
        const std::uint8_t* srcRow = srcPixels + y * srcPitch;
        for (int x = 0; x < srcW; ++x) {
            Uint8 r = 0, g = 0, b = 0, a = 0;
            read_rgba(srcRow + x * bpp, r, g, b, a);
            const Uint8 m = static_cast<Uint8>(std::max({r, g, b}));
            maxA = std::max(maxA, a);
            maxRGB = std::max(maxRGB, m);
            sumRGB += m;
        }
    }

    const bool useRGB = (maxA == 0);
    const double avgRGB = (pixelCount > 0) ? (static_cast<double>(sumRGB) / static_cast<double>(pixelCount)) : 0.0;
    const bool invertRGB = useRGB && (avgRGB > 127.0);

    for (int y = 0; y < srcH; ++y) {
        const int oy = dstY + y;
        if (oy < 0 || oy >= out.h) {
            continue;
        }

        const std::uint8_t* srcRow = srcPixels + y * srcPitch;
        for (int x = 0; x < srcW; ++x) {
            const int ox = dstX + x;
            if (ox < 0 || ox >= out.w) {
                continue;
            }

            Uint8 r = 0, g = 0, b = 0, a = 0;
            read_rgba(srcRow + x * bpp, r, g, b, a);

            Uint8 coverage = useRGB ? static_cast<Uint8>(std::max({r, g, b})) : a;
            if (useRGB && invertRGB) {
                coverage = static_cast<Uint8>(255 - coverage);
            }

            out.pixels[static_cast<std::size_t>(oy) * static_cast<std::size_t>(out.w) + static_cast<std::size_t>(ox)] = coverage;
        }
    }

    if (locked) {
        SDL_UnlockSurface(rgba);
    }

    SDL_DestroySurface(rgba);

    return out;
}
