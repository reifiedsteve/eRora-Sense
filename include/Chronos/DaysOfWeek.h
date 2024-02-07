#pragma once

#include "Chronos/DayOfWeek.h"
#include "Utils/Parser.h"
#include "Utils/UpperLower.h"
#include "Utils/Trimmer.h"

#include <string>

class DaysOfWeek
{
public:

    typedef DayOfWeek Day;

    DaysOfWeek() : _dayBits(0) {}
    
    void add(DayOfWeek day);
    bool has(DayOfWeek day) const;

    static bool tryParse(const std::string& daysStr, DaysOfWeek& days);

    std::string str() const;

private:

    static void _setBit(uint8_t& val, int bitNo);
    static void _clrBit(uint8_t& val, int bitNo);
    static void _writeBit(uint8_t& val, int bitNo, bool set);
    static bool _testBit(uint8_t val, int bitNo);

    uint8_t _dayBits;
};
