#pragma once

#include <stdint.h>

class ParticleObserver
{
public:

    virtual void onParticleReading(uint16_t pm01, uint16_t pm25, uint16_t pm10) {}
};
