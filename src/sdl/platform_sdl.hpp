#pragma once
#include "platform.hpp"

class SdlPlatform final : public IPlatform {
  public:
    explicit SdlPlatform(const AppArgs &args);
    ~SdlPlatform() override;

    void run() override;

  private:
    AppArgs m_args;
};
