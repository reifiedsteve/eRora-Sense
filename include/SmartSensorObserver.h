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

    virtual void onSwitchOnOff(bool on) {}
    virtual void onFanSpeed(int speed) {}

    virtual void onTriggerInspectionLight() {}

    virtual void onCabinetBrightness(uint8_t brightness) {}
    virtual void onCabinetColour(uint8_t hue, uint8_t sat) {}

    // virtual void onDisplayMode(DisplayMode mode) = 0;
    
    virtual void onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) {}
    virtual void onFPS(unsigned fps) {}
    
    /// @brief Inform when IAQ is available (or not).
    /// @param available Returns true if available; otherwise false.
    virtual void onIAQAvailability(bool available) {}

    /// @brief Inform of a change in IAQ level.
    /// @param present IAQ level. AQI 0-500.
    virtual void onIAQ(float iaq) {}

    /// @brief Inform of a change in TVOC level (in parts per billion).
    /// @param present Returns total TVOC in ppm.
    virtual void onTVOC(float tvoc) {}

    /// @brief Inform of a change in CO2 level (in total parts-per-million).
    /// @param present Returns CO2 in ppm. 
    virtual void onCO2(float co2) {}

    /// @brief Inform of a change in temperature.
    /// @param temperature The temperture in C. 
    virtual void onTemperature(float temperature) {}

    /// @brief Inform of a change in humidity.
    /// @param humidity The humidity as a percentage. 
    virtual void onHumidity(float humidity) {}

    /// @brief Inform of a change in air pressure.
    /// @param pressure The air pressure in hPa. 
    virtual void onAirPressure(float hPa) {}

    /// @brief Observe the current particulate matter for PM 1.0.
    /// @param pm01 PM1.0 levels (in ug/m3). 
    virtual void onPM01(uint16_t pm01) {}

    /// @brief Observe the current particulate matter for PM 2.5.
    /// @param pm01 PM2.5 levels (in ug/m3). 
    virtual void onPM25(uint16_t pm25) {}

    /// @brief Observe the current particulate matter for PM 10.0.
    /// @param pm01 PM10.0 levels (in ug/m3). 
    virtual void onPM10(uint16_t pm10) {}

    // Allow the observer to know that we are informing them of 
    // a block of updates in one go, and when we have finished
    // doing so.

    virtual void onGroupUpdateBegins() {}
    virtual void onGroupUpdateEnds() {}
    
private:

    const char* _observerName;
};

