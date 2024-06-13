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
    , _loopPeriod(TimeSpan::fromMilliseconds(100))
    , _loopTimer(_loopPeriod.millis(), CountdownTimer::State::Running)
{
    _leds.resize(_noOfLEDs);
}

void CabinetLights::setColour(const CRGB& rgb) {
    _colour = rgb;
}

void CabinetLights::setIlluminationColour(const CRGB& rgb) {
    _inspectionColour = rgb;
}

void CabinetLights::setInspectionTime(const TimeSpan& interval) {
    _inspectionPeriod = interval;
}

void CabinetLights::setPower(bool on) {
    _power = on;
}

void CabinetLights::triggerInspection() {
    _inspecting = true;
    _inspectionTimer.restart();
}

void CabinetLights::setup() {
    FastLED.addLeds<WS2812B, LedsDataPin>(&_leds[0], _noOfLEDs);
    // TODO: make the following less hard-coded.
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 1200); // 1200mA is conservative for 30-LEDs (wth max 500mA overhead for sensors/fan/display etc).
    FastLED.setBrightness(255);
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
        colour = _inspecting ? _inspectionColour : _colour;

        if (_inspecting && _inspectionTimer.hasExpired()) {
            _inspecting = false;
        }
    }
}

