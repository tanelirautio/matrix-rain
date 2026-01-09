#include "app.hpp"
#include "args.hpp"
#include <iostream>


int main(int argc, char *argv[]) {
    AppArgs args = parseArgs(argc, argv);

    try {
        App app(args);
        app.run();
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
