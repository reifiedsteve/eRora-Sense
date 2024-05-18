#pragma once

class HumiditySensor {
public:

/*
    /// @brief Is the humidity sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    virtual bool isHumiditySensorReady() = 0;
*/
    /// @brief Check whether a relative humidity measurement is available.
    /// @return Return true if available. Otherwise false. 
    virtual bool isHumidityAvailable() = 0;

    /// @brief Read humidity.
    /// @return Humidity in units of hPa.
    virtual float readHumidity() = 0;
};
