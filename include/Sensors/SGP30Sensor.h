#pragma once

#include "Sensors/TVOCSensor.h"
#include "Sensors/CO2Sensor.h"
#include "Sensors/HydrogenSensor.h"
#include "Sensors/EthenolSensor.h"

#include "Chronos/Stopwatch.h"

#include <Adafruit_SGP30.h>

class SGP30Sensor
  : public TVOCSensor
  , public CO2Sensor
  , public HydrogenSensor
  , public EthenolSensor
{
public:

    explicit SGP30Sensor(TwoWire& i2cBus = Wire);

    void setup();
    void loop();

    uint16_t readTVOC() override;
    uint16_t readCO2() override; // Actually eCO2 rather than CO2 specifically.
    uint16_t readHydrogen() override;
    uint16_t readEthenol() override;

    bool calibrateBaseline();

private:

    TwoWire& _i2cBus;
    
    Adafruit_SGP30 _sensor;
    Stopwatch _timer;

    uint16_t _tvoc, _eco2;
    Stopwatch::Ticks _timeRead;

    uint16_t _hydrogen, _ethenol;
    Stopwatch::Ticks _timeReadRaw;
};

