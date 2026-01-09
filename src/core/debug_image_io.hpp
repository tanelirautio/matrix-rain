#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace matrix_rain {

    bool writePGM(const std::string& path, int w, int h, const std::vector<std::uint8_t>& a8);

} // namespace matrix_rain