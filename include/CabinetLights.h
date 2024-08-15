#pragma once

#include <FastLED.h>

#include "Chronos/TimeSpan.h"
#include "Chronos/CountdownTimer.h"

#include "PinAssignments.h"

#include <functional>

class CabinetLights
{
public:

    static const uint8_t LedsDataPin = PinAssignments::CabinetLightsDataPin;

    explicit CabinetLights(size_t noOfLEDs);

    void setColour(const CRGB& rgb);
    void setInspectionColour(const CRGB& rgb);
    void setInspectionTime(const TimeSpan& interval);
    void setMaximumBrightness(int brightness);
    void setCurrentLimit(unsigned mA);

    void setPower(bool on);

    void triggerInspectionLight();

    typedef std::function<void(bool onOff)> ObserverFunc;
    void observeInspectionLight(ObserverFunc func);

    void setup();
    void loop();

private:

    void _setLedsColour();

    void _notifyObserver(bool onOff);
    
    size_t _noOfLEDs;
    std::vector<CRGB> _leds;
    CRGBSet _ledSet;

    uint8_t _dataPinNo;

    CRGB _colour;
    CRGB _inspectionColour;

    bool _power;
    bool _inspecting;

    TimeSpan _inspectionPeriod;
    CountdownTimer _inspectionTimer;
    ObserverFunc _observerFunc;

    TimeSpan _loopPeriod;
    CountdownTimer _loopTimer;
};