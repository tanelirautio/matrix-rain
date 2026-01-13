#pragma once

#if defined(__EMSCRIPTEN__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#define MATRIX_RAIN_SDL2 1
#else
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#define MATRIX_RAIN_SDL2 0
#endif

#if MATRIX_RAIN_SDL2
using MatrixRainPixelFormat = Uint32;
#else
using MatrixRainPixelFormat = SDL_PixelFormat;
#endif

inline bool matrix_rain_sdl_init_video() {
#if MATRIX_RAIN_SDL2
    return SDL_Init(SDL_INIT_VIDEO) == 0;
#else
    return SDL_Init(SDL_INIT_VIDEO);
#endif
}

inline bool matrix_rain_ttf_init() {
#if MATRIX_RAIN_SDL2
    return TTF_Init() == 0;
#else
    return TTF_Init();
#endif
}

inline SDL_Window *matrix_rain_create_window(const char *title, int w, int h, Uint64 flags) {
#if MATRIX_RAIN_SDL2
    return SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, static_cast<Uint32>(flags));
#else
    return SDL_CreateWindow(title, w, h, flags);
#endif
}

inline SDL_Renderer *matrix_rain_create_renderer(SDL_Window *window) {
#if MATRIX_RAIN_SDL2
    return SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
#else
    return SDL_CreateRenderer(window, nullptr);
#endif
}

inline void matrix_rain_get_window_size_pixels(SDL_Window *window, int *w, int *h) {
#if MATRIX_RAIN_SDL2
    SDL_GetWindowSize(window, w, h);
#else
    SDL_GetWindowSizeInPixels(window, w, h);
#endif
}

inline MatrixRainPixelFormat matrix_rain_rgba_pixel_format() {
#if MATRIX_RAIN_SDL2
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    return SDL_PIXELFORMAT_RGBA8888;
#else
    return SDL_PIXELFORMAT_ABGR8888;
#endif
#else
    return SDL_PIXELFORMAT_RGBA32;
#endif
}

inline SDL_Surface *matrix_rain_convert_surface(SDL_Surface *src, MatrixRainPixelFormat format) {
#if MATRIX_RAIN_SDL2
    return SDL_ConvertSurfaceFormat(src, format, 0);
#else
    return SDL_ConvertSurface(src, format);
#endif
}

inline void matrix_rain_destroy_surface(SDL_Surface *surface) {
    if (!surface) {
        return;
    }
#if MATRIX_RAIN_SDL2
    SDL_FreeSurface(surface);
#else
    SDL_DestroySurface(surface);
#endif
}

inline bool matrix_rain_lock_surface(SDL_Surface *surface) {
#if MATRIX_RAIN_SDL2
    return SDL_LockSurface(surface) == 0;
#else
    return SDL_LockSurface(surface);
#endif
}

inline bool matrix_rain_update_texture(SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch) {
#if MATRIX_RAIN_SDL2
    return SDL_UpdateTexture(texture, rect, pixels, pitch) == 0;
#else
    return SDL_UpdateTexture(texture, rect, pixels, pitch);
#endif
}

inline SDL_Texture *matrix_rain_create_texture(SDL_Renderer *renderer, MatrixRainPixelFormat format,
                                               SDL_TextureAccess access, int w, int h) {
    return SDL_CreateTexture(renderer, format, access, w, h);
}

inline void matrix_rain_render_texture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_FRect *src,
                                       const SDL_FRect *dst) {
#if MATRIX_RAIN_SDL2
    SDL_Rect srcRect{
        static_cast<int>(src->x),
        static_cast<int>(src->y),
        static_cast<int>(src->w),
        static_cast<int>(src->h),
    };
    SDL_Rect dstRect{
        static_cast<int>(dst->x),
        static_cast<int>(dst->y),
        static_cast<int>(dst->w),
        static_cast<int>(dst->h),
    };
    SDL_RenderCopy(renderer, texture, &srcRect, &dstRect);
#else
    SDL_RenderTexture(renderer, texture, src, dst);
#endif
}

inline void matrix_rain_render_fill_rect(SDL_Renderer *renderer, const SDL_FRect *rect) {
#if MATRIX_RAIN_SDL2
    SDL_Rect dstRect{
        static_cast<int>(rect->x),
        static_cast<int>(rect->y),
        static_cast<int>(rect->w),
        static_cast<int>(rect->h),
    };
    SDL_RenderFillRect(renderer, &dstRect);
#else
    SDL_RenderFillRect(renderer, rect);
#endif
}
