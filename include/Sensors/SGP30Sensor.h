#pragma once

#if 0

#include <functional>

#include "Sensors/TVOCSensor.h"
#include "Sensors/CO2Sensor.h"

#include "Chronos/Stopwatch.h"

#include <Adafruit_SGP30.h>

class SGP30Sensor
  : public TVOCSensor
  , public CO2Sensor
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
    inline void setSaveFunc(SaveFunc func) __attribute__((always_inline)) {
        _saveFunc = func;
    }

    /// @brief Provide the function for restoring the calibarated baseline.
    /// @param func The load function.
    inline void setLoadFunc(LoadFunc func) __attribute__((always_inline)) {
        _loadFunc = func;
    }

    /// @brief Post-construction intialisation.
    void setup();

    /// @brief Primary processing loop.
    void loop();

    /// @brief Provide the sensor with the current absolute humidity (mg/m^3).
    // While this is not strictly necessary, doing so will yield more accurate sensor readings. 
    /// (Note: an alternatvie method allows the caller to instead provide temperature
    /// and *relative* humidity, from which absolute humidity can be approcimated). 
    /// @param absoluteHumidity The absolute humidity in mg/m^3.
    inline void setAbsoluteHumidity(uint32_t absoluteHumidity) __attribute__((always_inline)) {
      _sensor.setHumidity(absoluteHumidity);
    }

    /// @brief Provide the sensor with the current temperature and relative humidity.
    /// Note: this is an alternative to provide absolute humidity (but serves the same purpose).
    /// @param tempC 
    /// @param relativeHumidityPercent 
    inline void setTemperatureAndRelativeHumidity(float tempC, float relativeHumidityPercent) __attribute__((always_inline)) {
      _sensor.setHumidity(_calculateAbsoluteHumidity(tempC, relativeHumidityPercent));
    }

    /// @brief Read the current TVOC (as parts-per-billion, ppm).
    /// @return The TVOC level (in ppb, range 0-60,000).
    float readTVOC() override;

    /// @brief Read the current eCO2 level (in parts-per-million).
    /// @return The CO2 level (in ppm, range 400-60,000). 
    float readCO2() override; // Actually eCO2 rather than CO2 specifically.


    /// @brief Read the current H2 (hydrogen molecule) level (raw values).
    /// These numbers are raw sensor values and meaningless without
    /// knowing their units or how they relate to actual gas presence
    /// (e.g. do they even respond linearly w.r.t gas amount?).
    /// @return H2 level (raw value from sensor).
    uint16_t readHydrogen();

    /// @brief Read the current ethernol level (raw values).
    /// These numbers are raw sensor values and meaningless without
    /// knowing their units or how they relate to actual gas presence
    /// (e.g. do they even respond linearly w.r.t gas amount?).
    /// @return H2 level (raw value from sensor).
    uint16_t readEthenol();

    /// @brief Calibrate the sensor. This should be done
    /// every 7-days in a "baseline" enviroment (i.e. clean air).
    /// @return The result of calibration.
    CalibrationResult calibrate();

    /// @brief Determine if the sensor is even connected.
    /// @return If connected, returns true; otherwise false;
    bool isConnected() const;

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
    bool _connected;

    SaveFunc _saveFunc;
    LoadFunc _loadFunc;

    Adafruit_SGP30 _sensor;
    bool _restoredCalibration;

    Stopwatch _timer;

    Stopwatch::Ticks _timeRead;
    Stopwatch::Ticks _timeReadRaw;
};

#endif
