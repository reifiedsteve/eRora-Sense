#pragma once

#include <sstream>
#include <iomanip>

#include "Diagnostics/Logging.h"

class TimeOfDay {
public:

    typedef uint8_t Unit;

    static bool tryParse(const std::string& str, TimeOfDay& time);
    
    TimeOfDay()
      : _hour(0), _minute(0)
    {}
    
    TimeOfDay(Unit hour, Unit minute)
      : _hour(hour), _minute(minute)
    {}

    Unit hour() const __attribute__((always_inline)) {
        return _hour;
    }

    Unit minute() const __attribute__((always_inline)) {
        return _minute;
    }

    bool operator==(const TimeOfDay& rhs) const {
        return _equal(rhs);
    };

    bool operator!=(const TimeOfDay& rhs) const {
        return !_equal(rhs);
    }

    std::string str() const {
        #if false
        std::ostringstream ss;
        if (_hour < 10) ss << "0";
        ss << (int)_hour << ":";
        if (_minute < 10) ss << "0";
        ss << (int)_minute;
        std::string str(ss.str());
        Log.verboseln("TimeOfDay: for hour %d and min %d returning \"%s\".", _hour, _minute, str.c_str());
        return str;
        #else
        std::ostringstream ss;
        ss << std::setfill('0') << std::setw(2) << (int) _hour << ":" << std::setfill('0') << std::setw(2) << (int)_minute;
        return ss.str();
        #endif
    }

private:

    bool _equal(const TimeOfDay& rhs) const {
        return (_hour == rhs._hour) && (_minute == rhs._minute);
    }

    Unit _hour, _minute;
};

