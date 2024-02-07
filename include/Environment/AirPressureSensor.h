#pragma once

class AirPressureSensor {
public:
/*
    /// @brief Check whether an air pressure measurement is available.
    /// @return Return true if available. Oherwise false. 
    virtual bool isAirPressureAvailable() = 0;
*/
    /// @brief Read air pressure.
    /// @return Air pressure in units of hPa.
    virtual float readAirPressure() = 0;
};
