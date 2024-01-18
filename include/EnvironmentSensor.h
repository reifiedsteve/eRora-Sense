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

class EnvironmentSensor
{
public:

    bool setup();

private:

    Adafruit_BME680 _sensor;
};