#include "Utils/Parser.h"

#include <sstream>

bool Parser::tryParse(const std::string& str, int& value, Base base) {
    std::stringstream is(str);
    if (base == Base::Hex) is >> std::hex;
    is >> value;
    return (is && !(is.bad()));
}

bool Parser::tryParse(const std::string& str, bool& state)
{
    bool handled(false);
    
    if ((str == "true") || (str == "on") || (str == "enable")|| (str == "enabled") || (str == "1")) {
        state = true;
        handled = true;
    } 

    else if ((str == "false") || (str == "off") || (str == "disable") || (str == "disabled") || (str == "0")) {
        state = false;
        handled = true;
    }

    return handled;
}

bool Parser::tryParse(const String& str, int& value, Base base) {
    return tryParse(std::string(str.c_str()), value, base);
}

bool Parser::tryParse(const String& str, bool& state) {
    return tryParse(std::string(str.c_str()), state);
}

#if false

bool Parser::tryParseRGB(const std::string& str, CRGB& rgb) {
    return tryParseRGB(String(str.c_str()), rgb);
}

bool Parser::tryParseRGB(const String& str, CRGB& rgb)
{
    uint8_t r, g, b;
    bool parsed(_tryParseTripleByte(str, r, g, b));

    if (parsed) {
        rgb = CRGB(r, g, b);
    }   

    return parsed;
}

#if false

bool Parser::_tryParseHSV(const std::string& str, CHSV& hsv) {
    return tryParseHSV(String(str.c_str()), hsv);
}

bool Parser::tryParseHSV(const String& str, CHSV& hsv)
{
    uint8_t h, s, v;
    bool parsed(_tryParseTripleByte(str, h, s, v));

    if (parsed) {
        hsv = CHSV(h, s, v);
    }

    return parsed;
}

#endif

bool Parser::startsWith(const std::string& str, const std::string& prefix) {
    return (str.length() >= prefix.length()) && (str.substr(0, prefix.length()) == prefix);
}

bool Parser::_tryParseTripleByte(const String& str, uint8_t& a, uint8_t& b, uint8_t& c) {
    return _tryParseTripleDecByte(str, a, b, c) || _tryParseTripleHexByte(str, a, b, c);
}

bool Parser::_tryParseTripleHexByte(const String& str, uint8_t& a, uint8_t& b, uint8_t& c)
{
    Log.verboseln("Parser:_tryParseTripleHexByte parsing \"%s\"", str.c_str());

    String byte3Str(str.startsWith("#") ? str.substring(1) : str);
    bool parsed(byte3Str.length() == 6);

    if (parsed) {
        long val = strtol(byte3Str.c_str(), nullptr, 16); 
        c = (uint8_t)(val & 0xFF);
        b = (uint8_t)((val >> 8) & 0xFF);
        a = (uint8_t)((val >> 16) & 0xFF);
    }

    return parsed;
}

bool Parser::_tryParseTripleDecByte(const String& str, uint8_t& a, uint8_t& b, uint8_t& c)
{
    Log.verboseln("Parser:_tryParseTripleDecByte parsing \"%s\"", str.c_str());

    bool parsed(false);

    if (str.indexOf(',') >= 0)
    {
        int x, y, z;
        char delim1, delim2;

        std::istringstream ss(str.c_str());
        ss >> x >> delim1 >> y >> delim2 >> z;

        if (ss && (delim1 == ',') && (delim2 == ',')) {
            if ((0 <= x) && (x <= 255) && (0 <= y) && (y <= 255) && (0 <= z) && (z <= 255)) {
                a = x;
                b = y;
                c = z;
                parsed = true;
            }
        }
    }

    return parsed;
}

bool Parser::_parseValue(const std::string& str, uint8_t& byte)
{
    bool parsed(false);

    int value;

    if(_parseValue(str, value)) {
        if ((0 <= value) && (value <= 255)) {
            byte = (uint8_t)value;
            parsed = true;
        }
    }
    
    return parsed;
}

bool Parser::_parseValue(const std::string& str, int8_t& byte)
{
    bool parsed(false);

    int value;

    if(_parseValue(str, value)) {
        if ((-128 <= value) && (value <= 127)) {
            byte = (int8_t)value;
            parsed = true;
        }
    }

    return parsed;
}

bool Parser::_parseValue(const std::string& str, int& value)
{
    bool parsed(false);
    size_t len(str.size());

    if (len > 0)
    {
        size_t start(0);
        bool negative(false);

        if (str[0]=='-') {
            start = 1;
            negative = true;
        }
        
        else if (str[0]=='+') {
            start = 1;
            negative = false;
        }

        if ((len - start) > 0)
        {
            std::string digits(str.substr(start));

            if (Parser::_allDigits(digits)) {
                value = atoi(digits.c_str());
                value *= (negative ? -1 : 1);
                parsed = true;
            }
        }
    }

    return parsed;
}

bool Parser::_parseValue(const std::string& str, bool& state)
{
    bool parsed(false);

    if (Parser::_meansTrue(str)) {
        state = true;
        parsed = true;
    }

    else if (Parser::_meansFalse(str)) {
        state = false;
        parsed = true;
    }

    return parsed;
}


bool Parser::_allDigits(const std::string& str) {
    return str.find_first_not_of("0123456789") == std::string::npos;
}

bool Parser::_meansTrue(const std::string& str) {
    return (str == "true") || (str == "on") || (str == "enable") || (str == "enabled") || (str == "1");
}

bool Parser::_meansFalse(const std::string& str) {
    return (str == "false") || (str == "off") || (str == "disable")|| (str == "disabled") || (str == "0");
}

#endif


