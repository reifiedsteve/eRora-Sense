#include "CabinetLights.h"

CabinetLights::CabinetLights(size_t noOfLEDs)
    : _noOfLEDs(noOfLEDs)
    , _leds(noOfLEDs)
    , _ledSet(_leds.data(), 0, noOfLEDs)
    , _colour(0, 0, 25)
    , _inspectionColour(255, 255, 255)
    , _power(false)
    , _inspecting(false)
    , _inspectionPeriod(TimeSpan::fromSeconds(30))
    , _inspectionTimer(_inspectionPeriod.millis())
    , _observerFunc()
    , _loopPeriod(TimeSpan::fromMilliseconds(50)) // Static colours so doesn't need to be fast.
    , _loopTimer(_loopPeriod.millis(), CountdownTimer::State::Running)
{
    _leds.resize(_noOfLEDs);
}

void CabinetLights::setColour(const CRGB& rgb) {
    _colour = rgb;
}

void CabinetLights::setInspectionColour(const CRGB& rgb) {
    _inspectionColour = rgb;
}

void CabinetLights::setInspectionTime(const TimeSpan& interval) {
    _inspectionPeriod = interval;
    _inspectionTimer.setCountdown(_inspectionPeriod.millis());
}

void CabinetLights::setMaximumBrightness(int brightness) {
    FastLED.setMaximumBrightness(brightness);
}

void CabinetLights::setCurrentLimit(unsigned mA) {
    FastLED.setMaxPowerInVoltsAndMilliamps(5, mA); // 1200mA is conservative for 30-LEDs (wth max 500mA overhead for sensors/fan/display etc).
}

void CabinetLights::setPower(bool on) {
    _power = on;
    if (!_power) {
        _inspecting = false;
        _notifyObserver(_inspecting);
    }
}

void CabinetLights::triggerInspectionLight() {
    if (_power) {
        if (!_inspecting) {
            _inspecting = true;
            _notifyObserver(_inspecting);
        }
        _inspectionTimer.restart();
    }
}

void CabinetLights::observeInspectionLight(ObserverFunc func) {
    _observerFunc = func;
    _notifyObserver(_inspecting);
}

void CabinetLights::setup() {
    FastLED.addLeds<WS2812B, LedsDataPin>(&_leds[0], _noOfLEDs);    
    // Note: notionally, using a 12V 2A PSU, with a buck converter of 5V output max 3A. So limited by the 2A.
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 1200); // 1200mA is conservative for 30-LEDs (wth max 500mA overhead for sensors/fan/display etc).
    FastLED.setMaximumBrightness(255);
}

void CabinetLights::loop()
{
    if (_loopTimer.hasExpired()) {
        _setLedsColour();
        FastLED.show();
        _loopTimer.restart();
    }
}

void CabinetLights::_setLedsColour() 
{
    CRGB colour(CRGB::Black);

    if (_power) 
    {
        if (_inspecting && _inspectionTimer.hasExpired()) {
            _inspecting = false;
            _notifyObserver(_inspecting);
        }

        colour = _inspecting ? _inspectionColour : _colour;
    }

    else {
        if (_inspecting) {
            _inspectionTimer.expire();
            _inspecting = false;
            _notifyObserver(_inspecting);
        }
    }

    _ledSet = colour;
}

void CabinetLights::_notifyObserver(bool onOff) {
    if (_observerFunc) {
        _observerFunc(_inspecting);
    }
}


