#pragma once
#include "args.hpp"
#include <memory>

class IPlatform {
  public:
    virtual ~IPlatform() = default;
    virtual void run() = 0;
};

std::unique_ptr<IPlatform> createPlatform(const AppArgs &args);
