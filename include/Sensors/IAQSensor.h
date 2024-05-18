#pragma once

class IAQSensor {
public:

/*
    /// @brief Is the IAQ sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    virtual bool isIAQSensorReady() = 0;
*/
    /// @brief Check whether a humidity measurement is available.
    /// @return Return true if available. Otherwise false. 
    virtual bool isIAQAvailable() = 0;

    /// @brief Read humidity.
    /// @return Humidity in units of hPa.
    virtual float readIAQ() = 0;
};
