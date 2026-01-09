#include "args.hpp"
#include <iostream>

namespace {
    std::string helpText() {
        return "Usage: app [options]\n"
               "Options:\n"
               "  --fullscreen, -f               Start in fullscreen mode\n"
               "  --windowed, -w                 Start in windowed mode (default)\n"
               "  --width <pixels>, -W <pixels>  Set window width (windowed mode)\n"
               "  --height <pixels>, -H <pixels> Set window height (windowed mode)\n"
               "  --cell <pixels>                Set square cell size in pixels\n"
               "  --cell-width <pixels>          Set cell width in pixels\n"
               "  --cell-height <pixels>         Set cell height in pixels\n"
               "  --fontSize <points>            Override font size in points\n"
               "  --seed <uint>                  Use fixed RNG seed\n"
               "  --help, -h                     Show this help message\n";
    }
} // namespace

AppArgs parseArgs(int argc, char* argv[]) {
    ArgsParseResult result = parseArgsEx(argc, argv);
    if (result.shouldExit) {
        if (!result.error.empty()) {
            std::cerr << result.error << "\n";
        }
        if (result.showHelp) {
            std::cout << helpText();
        }
        std::exit(result.exitCode);
    }

    return result.args;
}

ArgsParseResult parseArgsEx(int argc, char* argv[]) {
    ArgsParseResult result{};
    AppArgs& args = result.args;

    if (argc <= 1) {
        args.mode = StartMode::Fullscreen;
        args.width = 800;
        args.height = 600;
        args.widthProvided = true;
        args.heightProvided = true;
        return result;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--fullscreen" || arg == "-f") {
            args.mode = StartMode::Fullscreen;
        } else if (arg == "--windowed" || arg == "-w") {
            args.mode = StartMode::Windowed;
        } else if ((arg == "--width" || arg == "-W") && i + 1 < argc) {
            args.width = std::stoi(argv[++i]);
            args.widthProvided = true;
        } else if ((arg == "--height" || arg == "-H") && i + 1 < argc) {
            args.height = std::stoi(argv[++i]);
            args.heightProvided = true;
        } else if (arg == "--cell" && i + 1 < argc) {
            int px = std::stoi(argv[++i]);
            if (px > 0) {
                args.cellWidthPx = px;
                args.cellHeightPx = px;
                args.cellWidthProvided = true;
                args.cellHeightProvided = true;
            }
        } else if (arg == "--cell-width" && i + 1 < argc) {
            int px = std::stoi(argv[++i]);
            if (px > 0) {
                args.cellWidthPx = px;
                args.cellWidthProvided = true;
            }
        } else if (arg == "--cell-height" && i + 1 < argc) {
            int px = std::stoi(argv[++i]);
            if (px > 0) {
                args.cellHeightPx = px;
                args.cellHeightProvided = true;
            }
        } else if (arg == "--fontSize" && i + 1 < argc) {
            args.fontSizePt = std::stof(argv[++i]);
            args.fontSizeProvided = (args.fontSizePt > 0.0f);
        } else if (arg == "--seed" && i + 1 < argc) {
            args.rngSeed = static_cast<std::uint32_t>(std::stoul(argv[++i]));
            args.useFixedSeed = true;
        } else if (arg == "--debugDumps") {
            args.debugDumps = true;
        } else if ((arg == "--help" || arg == "-h") || (arg == "--usage") || (arg == "-u")) {
            result.shouldExit = true;
            result.showHelp = true;
            result.exitCode = 0;
            return result;
        } else if (!arg.empty() && arg[0] == '-') {
            result.shouldExit = true;
            result.showHelp = true;
            result.exitCode = 1;
            result.error = "Unknown option: " + arg;
            return result;
        }
    }

    return result;
}
