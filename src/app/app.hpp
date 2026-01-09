#pragma once
#include "args.hpp"

class App {
  public:
    explicit App(const AppArgs &args);
    void run();

  private:
    AppArgs m_args;
};
