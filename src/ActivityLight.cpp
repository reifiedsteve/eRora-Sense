#include "ActivityLight.h"

ActivityLight::ActivityLight(OnboardLEDBlinker& blinker, int transientOnTimeMs)
    : SmartSensorObserver("activity light")
    , _onTimeMs(transientOnTimeMs)
    , _offTimeMs(50)
    , _blinker(blinker)
{}

void ActivityLight::onSwitchOnOff(bool on) {
    _blink();
}

void ActivityLight::onFanSpeed(int speed) {
    _blink();
}

void ActivityLight::onBacklightBrightness(uint8_t brightness) {
    _blink();
}

void ActivityLight::onBacklightColour(uint8_t hue, uint8_t sat) {
    _blink();
}

// void ActivityLight::onDisplayMode(DisplayMode mode) {
//    _blink();
//}

void ActivityLight::_blink() {
    _blinker.start(1, _onTimeMs, _offTimeMs);
}

void ActivityLight::onTemperature(float temperature) {
    // Only showing cotrol activity, so nothing to do here.
}

void ActivityLight::onHumidity(float humidity) {
    // Only showing cotrol activity, so nothing to do here.
}

void ActivityLight::onAirPressure(float hPa) {
    // Only showing cotrol activity, so nothing to do here.
}

void ActivityLight::onTVOC(float tvoc) {
    // Only showing cotrol activity, so nothing to do here.
}

void ActivityLight::onCO2(float co2) {
    // Only showing cotrol activity, so nothing to do here.
}

void ActivityLight::onIAQAvailability(bool available) {
    // Only showing cotrol activity, so nothing to do here.
}

void ActivityLight::onIAQ(float iaq) {
    // Only showing cotrol activity, so nothing to do here.
}
