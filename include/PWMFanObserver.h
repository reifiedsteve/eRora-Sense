#pragma once

#include <stdint.h>

class PWMFanObserver
{
public:

    virtual void onFanSpeed(uint8_t percentage) = 0;
};
