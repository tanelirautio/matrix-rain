#include "app.hpp"
#include "platform.hpp"

App::App(const AppArgs &args) : m_args(args) {}

void App::run() {
    auto platform = createPlatform(m_args);
    platform->run();
}
