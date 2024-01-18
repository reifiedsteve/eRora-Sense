#pragma once

//#include <Wire.h>
//#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

// Sensor types to support (more widely):
//    temperature
//    humidity
//    VoC / IAQ.
//    luminance.
//    human presense.
//    CO2 levels?
// NOte: LD2410 devices can hit 48C, so quite hot! 
// Hence need isolating from any temo sensor.

class EnvironmentSensor
{
public:

    bool setup();

private:

    Adafruit_BME680 _sensor;
};