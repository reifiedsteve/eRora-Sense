#include "Chronos/DaysOfWeek.h"

void DaysOfWeek::add(DayOfWeek day) {
    _setBit(_dayBits, (int)day);
}

bool DaysOfWeek::has(DayOfWeek day) const {
    return _testBit(_dayBits, (int)day);
}

bool DaysOfWeek::tryParse(const std::string& daysStr, DaysOfWeek& days)
{
    bool parsed(false);
    std::string str(UpperLower::toUpper(Trimmer::trim(daysStr)));

    size_t n(str.size());
    // Log.verboseln("DaysOfWeek: parsing \"%s\" (of size %d).", str.c_str(), n);

    if (n == 7) {
        days._dayBits = 0;
        _writeBit(days._dayBits, (int)Day::Sat, (str[0] == 'S'));
        _writeBit(days._dayBits, (int)Day::Sun, (str[1] == 'S'));
        _writeBit(days._dayBits, (int)Day::Mon, (str[2] == 'M'));
        _writeBit(days._dayBits, (int)Day::Tue, (str[3] == 'T'));
        _writeBit(days._dayBits, (int)Day::Wed, (str[4] == 'W'));
        _writeBit(days._dayBits, (int)Day::Thu, (str[5] == 'T'));
        _writeBit(days._dayBits, (int)Day::Fri, (str[6] == 'F'));
        parsed = true;
    }

    if (parsed) {
        // Log.verboseln("DaysOfWeek: parsed \"%s\" to %d.", daysStr.c_str(), days._dayBits);
    } else {
        Log.warningln("DaysOfWeek: failed to parse \"%s\".", daysStr.c_str(), days._dayBits);
    }

    return parsed;
}

std::string DaysOfWeek::str() const {
    std::stringstream ss;
    ss << (_testBit(_dayBits, (int)Day::Sat) ? "S" : "-");
    ss << (_testBit(_dayBits, (int)Day::Sun) ? "S" : "-");
    ss << (_testBit(_dayBits, (int)Day::Mon) ? "M" : "-");
    ss << (_testBit(_dayBits, (int)Day::Tue) ? "T" : "-");
    ss << (_testBit(_dayBits, (int)Day::Wed) ? "W" : "-");
    ss << (_testBit(_dayBits, (int)Day::Thu) ? "T" : "-");
    ss << (_testBit(_dayBits, (int)Day::Fri) ? "F" : "-");
    return ss.str();
}

void DaysOfWeek::_setBit(uint8_t& val, int bitNo) {
    val |= (1 << ((uint8_t)bitNo)); 
}

void DaysOfWeek::_clrBit(uint8_t& val, int bitNo) {
    val &= ~(1 << ((uint8_t)bitNo)); 
}

void DaysOfWeek::_writeBit(uint8_t& val, int bitNo, bool set) {
    uint8_t oldVal(val);
    if (set) {
        _setBit(val, bitNo); 
    } else {
        _clrBit(val, bitNo); 
    }
    // Log.verboseln("DaysOfWeek: For %d, writing bit %d to %s = %d.", (int)oldVal, bitNo, (set ? "true" : "false"), val);
}

bool DaysOfWeek::_testBit(uint8_t val, int bitNo) {
    return ((val & (1 << ((uint8_t)bitNo))) > 0);
}
