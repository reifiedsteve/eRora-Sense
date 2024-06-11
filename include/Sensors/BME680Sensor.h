#pragma once

#include "Sensors/TemperatureSensor.h"
#include "Sensors/HumiditySensor.h"
#include "Sensors/AirPressureSensor.h"
#include "Sensors/TVOCSensor.h"
#include "Sensors/CO2Sensor.h"
#include "Sensors/IAQSensor.h"

#include "Chronos/RetryFunc.h"

#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

#include "Diagnostics/Logging.h"

#include <bsec.h>

class BME680Sensor
  : public TemperatureSensor
  , public HumiditySensor
  , public AirPressureSensor
  , public TVOCSensor
  , public CO2Sensor
  , public IAQSensor
{
public:

    /// @brief Construct a BME680 sensor.
    /// @param sampleInterval The sample interval. (Note: Shorter periods accelerate sensor wear.)
    explicit BME680Sensor(const TimeSpan& sampleInterval = TimeSpan::fromSeconds(5));

    /// @brief Perform any post-contruction intiaiisation.
    void setup();

    /// @brief Perform measurements,
    void loop();

    /// @brief Inquire on whether the sensor is connected/operational.
    /// @return If connected, returns true; otherwise false.
    bool connected();

    /*
    /// @brief Is the temperature sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    bool isTemperatureSensorReady() override;
    */

    /// @brief Check whether a temperature measurement is available.
    /// @return Return true if available. Otherwise false. 
    bool isTemperatureAvailable() override;

    /// @brief Returns the most recent ambient temperature measurement. 
    /// @return The temperature in C.
    float readTemperature() override;

    /*
    /// @brief Is the humidity sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    bool isHumiditySensorReady() override;
    */

    /// @brief Check whether a relative humidity measurement is available.
    /// @return Return true if available. Otherwise false. 
    bool isHumidityAvailable() override;

    /// @brief Returns the most recent relative humidity measurement.
    /// @return Relative humidity as a percentage.
    float readHumidity() override;

    /*
    /// @brief Is the airr-pressure sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    bool isAirPressureSensorReady() override;
    */

    /// @brief Check whether an air pressure measurement is available.
    /// @return Return true if available. Otherwise false. 
    bool isAirPressureAvailable() override;

    /// @brief Returns the most recent barometric pressure measurement.
    /// @return Returns barometric pressure in Pascals.
    float readAirPressure() override;

    /*
    /// @brief Is the TVOC sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    bool isTVOCSensorReady() override;
    */

    /// @brief Check whether a TVOC measurement is available.
    /// @return Return true if available. Otherwise false. 
    bool isTVOCAvailable() override;

    /// @brief Returns the most recent TVOC measurement.
    /// @return TVOC in parts-per-million (ppm).
    float readTVOC() override;

    /*
    /// @brief Is the CO2 sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    bool isCO2SensorReady() override;
    */

    /// @brief Check whether a TVOC measurement is available.
    /// @return Return true if available. Otherwise false. 
    bool isCO2Available() override;

    /// @brief Read ambient CO2 levels in ppm.
    /// @return CO2 level in ppm.
    float readCO2() override;

    /*
    /// @brief Is the IAQ sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    bool isIAQSensorReady() override;
    */

    /// @brief Check whether a humidity measurement is available.
    /// @return Return true if available. Otherwise false. 
    bool isIAQAvailable() override;


    /// @brief Returns the currwntly measured Indoor Air quality Index.
    /// @return The air quality index (see https://en.wikipedia.org/wiki/Air_quality_index).
    float readIAQ() override;

    /// @brief Set the calibrated temperature offset.
    /// @param offset The offset in degrees C.
    inline void setTemperatureOffset(float offset) __attribute__((always_inline)) {
        _tempOffset = offset;
    }

    /// @brief Set the calibrated relative humidity offset.
    /// @param offset The offset as a percentage.
    inline void setHumidityOffset(float offset) __attribute__((always_inline)) {
        _humidityOffset = offset;
    }

private:

    typedef RetryFunc _InitFunc;

    bool _init();
    bool _readMeasurements();

    Bsec _sensor;

    // _InitFunc _initialised;
    bool _initialised;
    CountdownTimer _timer;

    float _tempC, _humidity, _pressure, _eCO2, _tvoc, _iaq;
    bool _measured;

    float _tempOffset, _humidityOffset;
};
