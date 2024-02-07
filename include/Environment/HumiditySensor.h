#pragma once

class HumiditySensor {
public:
/*
    /// @brief Check whether a humidity measurement is available.
    /// @return Return true if available. Oherwise false. 
    virtual bool isHumidityAvailable() = 0;
*/
    /// @brief Read humidity.
    /// @return Humidity in units of hPa.
    virtual float readHumidity() = 0;
};
