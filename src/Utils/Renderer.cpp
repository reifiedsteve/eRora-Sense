#include "Utils/Renderer.h"
#include <sstream>
#include <iomanip>

std::string Utils::Renderer::render(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

#if 0

std::string Utils::Renderer::render(const CRGB& rgb, Base base)
{
    std::stringstream ss;

    if (base == Base::Hex) {
        ss  << std::setfill('0') << std::hex << std::uppercase 
            << std::setw(2) << rgb.r
            << std::setw(2) << rgb.g
            << std::setw(2) << rgb.b;
    }
    
    else {
        ss << rgb.r << "," << rgb.g << "," << rgb.b;
    }

    return ss.str();
}

#endif
