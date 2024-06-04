#pragma once

#include <inttypes.h>

class FanController 
{
public:

    /// @brief Set the fan speed.
    /// @param speed Speed of the fan, 0 -100;
    virtual void setFanSpeed(uint8_t speedPercentage) = 0;
};

