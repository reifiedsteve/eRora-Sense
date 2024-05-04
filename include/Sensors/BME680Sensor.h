#pragma once

#include "Sensors/TemperatureSensor.h"
#include "Sensors/HumiditySensor.h"
// #include "Sensors/AIrPressureSensor.h"
// #include "Sensors/GasLevelSensor.h"

#include "Chronos/RetryFunc.h"

#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

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

    explicit BME680Sensor(const TimeSpan& sampleInterval = TimeSpan::fromSeconds(5));

    /// @brief Perform any post-contruction intiaiisation.
    void setup();

    /// @brief Perform measurements,
    void loop();

    /// @brief Inquire on whether the sensor is connected/operational.
    /// @return If connected, returns true; otherwise false.
    bool connected();

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
    bool _readMeasurements();

    Adafruit_BME680 _sensor;

    // _InitFunc _initialised;
    bool _initialised;
    CountdownTimer _timer;

    float _tempC, _humidity, _pressure, _gas;
    bool _measured;
};
