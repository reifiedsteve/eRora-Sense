#pragma once

#include <sstream>
#include <Arduino.h> // for the awful String type.
/*
#include <FastLED.h> // for CRGB
*/
#include "Diagnostics/Logging.h"

class Parser
{
public:

    enum class Base : uint8_t {
        Dec, 
        Hex
    };

    static bool tryParse(const std::string& str, int& value, Base base = Base::Dec);
    static bool tryParse(const std::string& str, bool& state);

    static bool tryParse(const String& str, int& value, Base base = Base::Dec);
    static bool tryParse(const String& str, bool& state);

/*
    static bool tryParseRGB(const std::string& str, CRGB& rgb);
    static bool tryParseRGB(const String& str, CRGB& rgb);
*/
    static bool startsWith(const std::string& str, const std::string& prefix);
    
private:

    static bool _tryParseTripleByte(const String& str, uint8_t& a, uint8_t& b, uint8_t& c);
    static bool _tryParseTripleHexByte(const String& str, uint8_t& a, uint8_t& b, uint8_t& c);

    static bool _tryParseTripleDecByte(const String& str, uint8_t& a, uint8_t& b, uint8_t& c);

    static bool _parseValue(const std::string& str, uint8_t& byte);
    static bool _parseValue(const std::string& str, int8_t& byte);
    static bool _parseValue(const std::string& str, int& value);
    static bool _parseValue(const std::string& str, bool& state);

    static bool _allDigits(const std::string& str);    
    static bool _meansTrue(const std::string& str);
    static bool _meansFalse(const std::string& str);
};
