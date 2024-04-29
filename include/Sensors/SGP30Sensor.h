#pragma once

#include <functional>

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

    /// @brief The returned result of an attempted calibration.
    enum class CalibrationResult
    {
      /// @brief The calibration suceeded.
      OK, 

      /// @brief The calibration failed as the sensor has not been running a minimum of 12 hours.
      TooEarly,

      /// @brief The calibration was missing a save function to persist its result.
      NoSaveFunc,

      /// @brief The saving of calibration data failed.
      SaveFailed
    };

    /// @brief Initialise the sensor. 
    /// If any currebt calibrated baseline settings have been stored,
    /// then these will be restored and the sensor will be in a calibrated
    /// state from the start.
    /// @param i2cBus 
    explicit SGP30Sensor(TwoWire& i2cBus = Wire);

    /// @brief The save function for persisting the calibrated baseline.
    typedef std::function<bool(uint16_t tvoc, uint16_t co2)> SaveFunc;

    /// @brief The load function for restoring the calibrated baseline.
    typedef std::function<bool(uint16_t& tvos, uint16_t& co2)> LoadFunc;

    /// @brief Provide the function for persisting the calibrated baseline state.
    /// @param func The save function.
    void setSaveFunc(SaveFunc func) {
        _saveFunc = func;
    }

    /// @brief Provide the function for restoring the calibarated baseline.
    /// @param func The load function.
    void setLoadFunc(LoadFunc func) {
        _loadFunc = func;
    }

    /// @brief Post-construction intialisation.
    void setup();

    /// @brief Primary processing loop.
    void loop();

    void setAbsoluteHumidity(float tempC, uint32_t absoluteHumidity) {
      _sensor.setHumidity(absoluteHumidity);
    }

    void setTemperatureAndRelativeHumidity(float tempC, float relativeHumidityPercent) {
      _sensor.setHumidity(_calculateAbsoluteHumidity(tempC, relativeHumidityPercent));
    }

    uint16_t readTVOC() override;
    uint16_t readCO2() override; // Actually eCO2 rather than CO2 specifically.

    uint16_t readHydrogen() override;
    uint16_t readEthenol() override;

    /// @brief Calibrate the sensor. This should be done
    /// every 7-days in a "baseline" enviroment (i.e. clean air).
    /// @return The result of calibration.
    CalibrationResult calibrate();

    /// @brief Determine if the sensor is in a calibrated state.
    /// @return If calibrated then true; otherwise false;
    bool isCalibrated() const;

    /// @brief Inquire on the baseline (calibrated) TVOC.
    /// @return The baseline TVOC.
    uint16_t readBaselineTVOC();

    /// @brief Inquire on the baseline (calibrated) eCO2.
    /// @return The baseline CO2.
    uint16_t readBaselineCO2();

private:

    bool _restoreBaseline();

    bool _isSteady() const;

    static uint32_t _calculateAbsoluteHumidity(float temperature, float humidity);

    TwoWire& _i2cBus;

    SaveFunc _saveFunc;
    LoadFunc _loadFunc;

    Adafruit_SGP30 _sensor;
    bool _restoredCalibration;

    Stopwatch _timer;

    Stopwatch::Ticks _timeRead;
    Stopwatch::Ticks _timeReadRaw;
};

