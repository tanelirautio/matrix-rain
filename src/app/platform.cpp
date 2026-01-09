#include "platform.hpp"
#include "platform_sdl.hpp"

std::unique_ptr<IPlatform> createPlatform(const AppArgs &args) {
    return std::make_unique<SdlPlatform>(args);
}
