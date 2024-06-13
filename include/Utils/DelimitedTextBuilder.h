#pragma once

#include <string>
#include <sstream>

class DelimitedTextBuilder
{
public:

    explicit DelimitedTextBuilder(const std::string& delimiter)
      : _delimiter(delimiter)
      , _first(true)
    {}

    void append(const std::string& str) {
        _append(str.c_str());
    }

    void append(const char* str) {
        _append(str);
    }

    std::string str() {
        return _ss.str();
    }

    void clear() {
        _ss.clear();
        _first = true;
    }

private:

    void _append(const char* str) {
        if (_first) {
            _ss << _delimiter;
        }
        _ss << str;
        _first = false;
    }

    std::string _delimiter;

    bool _first;
    std::stringstream _ss;
};