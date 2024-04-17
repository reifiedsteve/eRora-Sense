#pragma once

class TemperatureSensor {
public:
/*
    /// @brief Check whether a temperature measurement is available.
    /// @return Return true if available. Oherwise false. 
    virtual bool isTemperatureAvailable() = 0;
*/
    /// @brief Read temperature.
    /// @return Temperature in Celcius.
    virtual float readTemperature() = 0;
};
