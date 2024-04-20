#pragma once

#include <stdint.h>

class CO2Sensor
{
public:

    /// @brief Read ambient CO2 levels in ppm.
    /// @return CO2 level in ppm.
    virtual uint16_t readCO2() = 0;
};