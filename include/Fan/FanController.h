#pragma once

#include <inttypes.h>

class FanController 
{
public:

    /// @brief Turn the fan on or off. When off, there is no fan motion. When switched on, it honours the currently set speed.
    /// @param on If true then turn the fan on; otherwise turns the fan off (i.e. it becomes motionless).
    virtual void setPower(bool on) = 0;

    /// @brief Set the fan speed.
    /// @param speed Speed of the fan, 0 -100. 0 is min (note: not fully off, just slowest speed), 100 is maximum speed.
    virtual void setFanSpeed(uint8_t speedPercentage) = 0;
};

