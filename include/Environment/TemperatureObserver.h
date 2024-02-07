#pragma once

class TemperatureObserver
{
public:

    /// @brief Inform of a change in temperature.
    /// @param temperature The temperture in C. 
    virtual void onTemperature(float temperature) = 0;
};
