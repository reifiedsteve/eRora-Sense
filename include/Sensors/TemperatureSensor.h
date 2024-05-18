#pragma once

class TemperatureSensor {
public:

/*
    /// @brief Is the temperature sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    virtual bool isTemperatureSensorReady() = 0;
*/
    /// @brief Check whether a temperature measurement is available.
    /// @return Return true if available. Otherwise false. 
    virtual bool isTemperatureAvailable() = 0;

    /// @brief Read temperature.
    /// @return Temperature in Celcius.
    virtual float readTemperature() = 0;
};
