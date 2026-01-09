#pragma once
#include <cstdint>
#include <string>

enum class StartMode { Windowed, Fullscreen };

struct AppArgs {
    StartMode mode = StartMode::Windowed;

    int width = 800; // used for windowed start
    int height = 600;

    bool widthProvided = false;
    bool heightProvided = false;

    int cellWidthPx = 32;
    int cellHeightPx = 32;
    bool cellWidthProvided = false;
    bool cellHeightProvided = false;

    float fontSizePt = 0.0f;
    bool fontSizeProvided = false;

    bool useFixedSeed = false;
    std::uint32_t rngSeed = 1;

    bool debugDumps = false;
};

struct ArgsParseResult {
    AppArgs args;
    bool shouldExit = false;
    bool showHelp = false;
    int exitCode = 0;
    std::string error;
};

AppArgs parseArgs(int argc, char *argv[]);
ArgsParseResult parseArgsEx(int argc, char *argv[]);
