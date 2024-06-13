#pragma once

#include <FastLED.h>

#include "Chronos/TimeSpan.h"
#include "Chronos/CountdownTimer.h"

#include "PinAssignments.h"

class CabinetLights
{
public:

    static const uint8_t LedsDataPin = PinAssignments::CabinetLightsDataPin;

    explicit CabinetLights(size_t noOfLEDs);

    void setColour(const CRGB& rgb);
    void setIlluminationColour(const CRGB& rgb);
    void setInspectionTime(const TimeSpan& interval);

    void setPower(bool on);

    void triggerInspection();

    void setup();
    void loop();

private:

    void _setLedsColour();

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

    TimeSpan _loopPeriod;
    CountdownTimer _loopTimer;
};
