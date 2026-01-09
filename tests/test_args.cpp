#include <catch2/catch_test_macros.hpp>

#include "args.hpp"

#include <vector>

static ArgsParseResult parseWithArgs(const std::vector<const char*>& argv) {
    return parseArgsEx(static_cast<int>(argv.size()), const_cast<char**>(argv.data()));
}

TEST_CASE("Args: defaults when no args provided") {
    const char* argv[] = {"app"};
    auto res = parseArgsEx(1, const_cast<char**>(argv));

    REQUIRE_FALSE(res.shouldExit);
    REQUIRE(res.args.mode == StartMode::Fullscreen);
    REQUIRE(res.args.width == 800);
    REQUIRE(res.args.height == 600);
    REQUIRE(res.args.widthProvided);
    REQUIRE(res.args.heightProvided);
    REQUIRE(res.args.cellWidthPx == 32);
    REQUIRE(res.args.cellHeightPx == 32);
    REQUIRE_FALSE(res.args.fontSizeProvided);
    REQUIRE_FALSE(res.args.useFixedSeed);
}

TEST_CASE("Args: width/height parsing") {
    auto res = parseWithArgs({"app", "--width", "1024", "--height", "768"});
    REQUIRE_FALSE(res.shouldExit);
    REQUIRE(res.args.width == 1024);
    REQUIRE(res.args.height == 768);
    REQUIRE(res.args.widthProvided);
    REQUIRE(res.args.heightProvided);
}

TEST_CASE("Args: mode flags") {
    auto fullscreen = parseWithArgs({"app", "--fullscreen"});
    REQUIRE_FALSE(fullscreen.shouldExit);
    REQUIRE(fullscreen.args.mode == StartMode::Fullscreen);

    auto windowed = parseWithArgs({"app", "--windowed"});
    REQUIRE_FALSE(windowed.shouldExit);
    REQUIRE(windowed.args.mode == StartMode::Windowed);
}

TEST_CASE("Args: cell size parsing") {
    auto res = parseWithArgs({"app", "--cell", "48"});
    REQUIRE_FALSE(res.shouldExit);
    REQUIRE(res.args.cellWidthPx == 48);
    REQUIRE(res.args.cellHeightPx == 48);
    REQUIRE(res.args.cellWidthProvided);
    REQUIRE(res.args.cellHeightProvided);

    auto resW = parseWithArgs({"app", "--cell-width", "40"});
    REQUIRE(resW.args.cellWidthPx == 40);
    REQUIRE(resW.args.cellWidthProvided);
    REQUIRE_FALSE(resW.args.cellHeightProvided);

    auto resH = parseWithArgs({"app", "--cell-height", "44"});
    REQUIRE(resH.args.cellHeightPx == 44);
    REQUIRE(resH.args.cellHeightProvided);
    REQUIRE_FALSE(resH.args.cellWidthProvided);
}

TEST_CASE("Args: font size parsing") {
    auto res = parseWithArgs({"app", "--fontSize", "12.5"});
    REQUIRE_FALSE(res.shouldExit);
    REQUIRE(res.args.fontSizeProvided);
    REQUIRE(res.args.fontSizePt == 12.5f);
}

TEST_CASE("Args: seed parsing") {
    auto res = parseWithArgs({"app", "--seed", "123"});
    REQUIRE_FALSE(res.shouldExit);
    REQUIRE(res.args.useFixedSeed);
    REQUIRE(res.args.rngSeed == 123u);
}

TEST_CASE("Args: unknown switch triggers help and non-zero exit") {
    auto res = parseWithArgs({"app", "-cell", "512"});
    REQUIRE(res.shouldExit);
    REQUIRE(res.showHelp);
    REQUIRE(res.exitCode == 1);
    REQUIRE(res.error.find("Unknown option") != std::string::npos);
}
