#pragma once

struct DeviceState
{
    DeviceState() 
      : power(false)
      , fanSpeed(1)
      , sensorReady(false)
      , heapTotal(0)
      , heapFree(0)
      , iaq(0)
      , tvoc(0)
      , co2(0)
      , temperature(0.0)
      , relHumidity(0.0)
      , airPressure(0)
      , pm01(0)
      , pm25(0)
      , pm10(0)
    {}

    // Device/control state.
    bool power;
    int fanSpeed;
    bool sensorReady;
    unsigned heapTotal, heapFree;

    // Sensor readings.
    int iaq, tvoc, co2;
    float temperature, relHumidity;
    int airPressure;

    int pm01, pm25, pm10;
};