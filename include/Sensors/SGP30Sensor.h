#pragma once

#include "Sensors/TVOCSensor.h"
#include "Sensors/CO2Sensor.h"

#include "Chronos/Stopwatch.h"

#include <Adafruit_SGP30.h>

class SGP30Sensor
  : public TVOCSensor
  , public CO2Sensor
{
public:

    SGP30Sensor();

    void setup();
    void loop();

    uint16_t readTVOC() override;
    uint16_t readCO2() override; // Actually eCO2 rather than CO2 specifically.

    bool calibrateBaseline();

private:

    Adafruit_SGP30 _sensor;
    Stopwatch _timer;

    uint16_t _tvoc, _eco2;
    Stopwatch::Ticks _timeRead;
};

