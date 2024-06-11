#pragma once

#include "SmartSensorObserver.h"
#include "Diagnostics/OnboardLEDBlinker.h"
#include "Chronos/TimeSpan.h"

class ActivityLight : public SmartSensorObserver
{
public:

    explicit ActivityLight(OnboardLEDBlinker& blinker, int transientOnTimeMs = 100);

    // We only respond to user instigated control events.
    // The activity light blinks to show that the sensor
    // has received some command event.
    
    virtual void onSwitchOnOff(bool on) override;
    virtual void onFanSpeed(int speed) override; 
    virtual void onCabinetBrightness(uint8_t brightness) override;
    virtual void onCabinetColour(uint8_t hue, uint8_t sat) override;
    // virtual void onDisplayMode(DisplayMode mode) override;

    void onTemperature(float temperature) override;
    void onHumidity(float humidity) override;
    void onAirPressure(float hPa) override;
    void onTVOC(float tvoc) override;
    void onCO2(float co2) override;
    void onIAQAvailability(bool available) override;
    void onIAQ(float iaq) override;

    void onPM01(uint16_t pm01) override;
    void onPM25(uint16_t pm25) override;
    void onPM10(uint16_t pm10) override;

    void onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) override;

private:

    void _blink();

    int _onTimeMs, _offTimeMs;
    OnboardLEDBlinker& _blinker;
};
