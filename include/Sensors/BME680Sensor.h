#pragma once

#include "Sensors/TemperatureSensor.h"
#include "Sensors/HumiditySensor.h"
// #include "Sensors/AIrPressureSensor.h"
// #include "Sensors/GasLevelSensor.h"

#include "Chronos/RetryFunc.h"

#include "Diagnostics/Logging.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

class BME680Sensor
  : public TemperatureSensor
  , public HumiditySensor
  // , public AirPressureSensor
  // , public GasLevelSensor
{
public:

    BME680Sensor();

    /// @brief Initialise the sensors.
    void setup();

    /// @brief Perform measurements,
    void loop();

    /// @brief Returns the most recent ambient temperature measurement. 
    /// @return The temperature in C.
    float readTemperature() override;

    /// @brief Returns the most recent relative humidity measurement.
    /// @return Relative humidity as a percentage.
    float readHumidity() override;

    /// @brief Returns the most tecent barometric pressure measurement.
    /// @return Returns barometric pressure in Pascals.
    float readAirPressure();

    /// @brief Returns the most recent VOC gas sensor resistance in Ohms.
    /// @return Gas sensor resistance in Ohms.
    float readGasLevel(); // Note: this is less accurate than that of TVOC from an SGP30.

private:

    typedef RetryFunc _InitFunc;

    bool _init();
    void _readMeasurements();

    Adafruit_BME680 _sensor;

    _InitFunc _initialised;

    float _tempC, _humidity, _pressure, _gas;
    bool _measured;
};
