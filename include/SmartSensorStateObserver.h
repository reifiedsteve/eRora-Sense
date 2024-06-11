#pragma once

#include <stdint.h>

class SmartSensorStateObserver 
{
public:

    explicit SmartSensorStateObserver(const char* observerName)
      : _observerName(observerName)
    {}

     inline const char* stateObserverName() const {
        return _observerName;
    }

    virtual void onSwitchOnOff(bool on) = 0;
    virtual void onFanSpeed(int speed) = 0;

    virtual void onCabinetBrightness(uint8_t brightness) = 0;
    virtual void onCabinetColour(uint8_t hue, uint8_t sat) = 0;

    // virtual void onDisplayMode(DisplayMode mode) = 0;
    
    virtual void onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) = 0;

private:

    const char* _observerName;
};

