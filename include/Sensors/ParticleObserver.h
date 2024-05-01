#pragma once

#include <stdint.h>

class ParticleObserver
{
public:

    /// @brief Observe the current particulate matter levels.
    /// @param pm01 PM1.0 levels (in ug/m3). 
    /// @param pm25 PM2.5 levels (in ug/m3).
    /// @param pm10 PM10.0 levels (in ug/m3).
    virtual void onParticleReading(uint16_t pm01, uint16_t pm25, uint16_t pm10) {}
};
