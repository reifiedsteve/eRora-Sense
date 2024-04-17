#pragma once

#include <Arduino.h>
#include <PMserial.h>

class ParticleSensor
{
public:

    struct Measurements {
        Measurements() : pm01(0), pm25(0), pm10(0) {}
        uint16_t pm01; // PM 1.0
        uint16_t pm25; // PM 2.5
        uint16_t pm10; // PM 10.0
    };

    /// @brief  Inquire on whether or not a new sample is available.
    /// @return True if available; otherwise false.
    virtual bool available() = 0;

    /// @brief Read the most recent sample.
    /// @param pm01 Level of PM 1.0 particulate matter.
    /// @param pm25 Level of PM 2.5 particulate matter.
    /// @param pm10 Level of PM 10.0 particulate matter.
    virtual const Measurements& read() = 0;
};

