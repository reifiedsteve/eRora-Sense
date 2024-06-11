#pragma once

#include <inttypes.h>

class SmartSensorObserver
{
public:

    SmartSensorObserver(const char* observerName)
      : _observerName(observerName)
    {}

     inline const char* observerName() const {
        return _observerName;
    }

    // Device/control monitoring.

    virtual void onSwitchOnOff(bool on) = 0;
    virtual void onFanSpeed(int speed) = 0;

    virtual void onCabinetBrightness(uint8_t brightness) = 0;
    virtual void onCabinetColour(uint8_t hue, uint8_t sat) = 0;

    // virtual void onDisplayMode(DisplayMode mode) = 0;
    
    virtual void onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) = 0;

    /// @brief Inform when IAQ is available (or not).
    /// @param available Returns true if available; otherwise false.
    virtual void onIAQAvailability(bool available) = 0;

    /// @brief Inform of a change in IAQ level.
    /// @param present IAQ level. AQI 0-500.
    virtual void onIAQ(float iaq) = 0;

    /// @brief Inform of a change in TVOC level (in parts per billion).
    /// @param present Returns total TVOC in ppm.
    virtual void onTVOC(float tvoc) = 0;


    /// @brief Inform of a change in CO2 level (in total parts-per-million).
    /// @param present Returns CO2 in ppm. 
    virtual void onCO2(float co2) = 0;

    /// @brief Inform of a change in temperature.
    /// @param temperature The temperture in C. 
    virtual void onTemperature(float temperature) = 0;

    /// @brief Inform of a change in humidity.
    /// @param humidity The humidity as a percentage. 
    virtual void onHumidity(float humidity) = 0;

    /// @brief Inform of a change in air pressure.
    /// @param pressure The air pressure in hPa. 
    virtual void onAirPressure(float hPa) = 0;

    /// @brief Observe the current particulate matter levels.
    /// @param pm01 PM1.0 levels (in ug/m3). 
    /// @param pm25 PM2.5 levels (in ug/m3).
    /// @param pm10 PM10.0 levels (in ug/m3).
    virtual void onParticleReading(uint16_t pm01, uint16_t pm25, uint16_t pm10) {}

    /// @brief Observe the current particulate matter for PM 1.0.
    /// @param pm01 PM1.0 levels (in ug/m3). 
    virtual void onPM01(uint16_t pm01) = 0;

    /// @brief Observe the current particulate matter for PM 2.5.
    /// @param pm01 PM2.5 levels (in ug/m3). 
    virtual void onPM25(uint16_t pm25) = 0;

    /// @brief Observe the current particulate matter for PM 10.0.
    /// @param pm01 PM10.0 levels (in ug/m3). 
    virtual void onPM10(uint16_t pm10) = 0;

private:

    const char* _observerName;
};

