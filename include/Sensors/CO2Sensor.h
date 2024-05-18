#pragma once

#include <stdint.h>

class CO2Sensor
{
public:

/*
    /// @brief Is the CO2 sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    virtual bool isCO2SensorReady() = 0;
*/
    /// @brief Check whether a TVOC measurement is available.
    /// @return Return true if available. Otherwise false. 
    virtual bool isCO2Available() = 0;

    /// @brief Read ambient CO2 levels in ppm.
    /// @return CO2 level in ppm.
    virtual float readCO2() = 0;
};