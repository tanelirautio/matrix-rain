#include "debug_image_io.hpp"

#include <fstream>

namespace matrix_rain {

    bool writePGM(const std::string& path, int w, int h, const std::vector<std::uint8_t>& a8) {
        if (w <= 0 || h <= 0) {
            return false;
        }
        if ((int)a8.size() != w * h) {
            return false;
        }

        std::ofstream f(path, std::ios::binary);
        if (!f) {
            return false;
        }

        // P5 = binary grayscale
        f << "P5\n" << w << " " << h << "\n255\n";
        f.write(reinterpret_cast<const char*>(a8.data()), a8.size());
        return (bool)f;
    }

} // namespace matrix_rain