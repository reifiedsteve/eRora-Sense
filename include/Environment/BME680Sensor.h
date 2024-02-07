#pragma once

#include "Environment/TemperatureSensor.h"
#include "Environment/HumiditySensor.h"
#include "Environment/AIrPressureSensor.h"
#include "Environment/GasLevelSensor.h"

#include "Diagnostics/Logging.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

class BME680Sensor
  : public TemperatureSensor
  , public HumiditySensor
  , public AirPressureSensor
  , public GasLevelSensor
{
public:

    void setup();
    void loop();

    float readTemperature() override;
    float readHumidity() override;
    float readAirPressure() override;
    float readGasLevel() override;

private:

    bool _init();
    void _readMeasurements();

    Adafruit_BME680 _sensor;

    float _tempC, _humidity, _pressure, _gas;

    bool _initialised;
    bool _measured;
};
