#pragma once

class AirPressureSensor {
public:

/*
    /// @brief Is the airr-pressure sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    virtual bool isAirPressureSensorReady() = 0;
*/
    /// @brief Check whether an air pressure measurement is available.
    /// @return Return true if available. Otherwise false. 
    virtual bool isAirPressureAvailable() = 0;

    /// @brief Read air pressure.
    /// @return Air pressure in units of hPa.
    virtual float readAirPressure() = 0;
};
