#pragma once

#include <string>
#include <sstream>

/*
#include <FastLED.h> // for CRGB
*/

#include "Diagnostics/Logging.h"

namespace Utils {

class Renderer
{
public:

    enum class Base : uint8_t {
        Dec, 
        Hex
    };

    static std::string render(int value);
    /*
    static std::string render(const CRGB& rgb, Base base = Base::Hex);
    */
    template <typename T>
    static std::string render(const T& obj) {
        std::stringstream ss;
        ss << obj;
        return ss.str();
    }
};

}