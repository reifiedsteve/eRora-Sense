#pragma once

#include <iomanip>

#include "Settings.h"

// #define PERSIST_CURRENT_STATE

class UserSettings : public Settings
{
public:

    // Note: we *avoid* peristing any "current" state and only
    // persist things explicitly saved by the user, as we wish
    // to void writing too often to the flash drive (as that
    // could cause premature failure of the drive).
    // i.e. it would be beneficial to e.g. persist the current on/off,
    // brightness etc, but that would mean saving them *every* time
    // they changed (including via any smart home logic), which
    // could be quite frequent (and more frequently than we
    // should probably write to the flash as it has limited write
    // lifespan).

    UserSettings(const char* settingsNamespace)
      : Settings(settingsNamespace)
    {}

    // Are there any user settings?

    inline void setCabinetLightColour(const CRGB& rgb) {
        _putSetting("led-r", (int)rgb.r);
        _putSetting("led-g", (int)rgb.g);
        _putSetting("led-b", (int)rgb.b);
    }

    inline CRGB getCabinetLightColour() const {
        uint8_t r = (uint8_t)_getSetting("led-r", 0);
        uint8_t g = (uint8_t) _getSetting("led-g", 0);
        uint8_t b = (uint8_t) _getSetting("led-b", 32);
        return CRGB(r, g, b);
    }

    inline void setCabinetInspectionLightColour(const CRGB& rgb) {
        _putSetting("ins-led-r", (int)rgb.r);
        _putSetting("ins-led-g", (int)rgb.g);
        _putSetting("ins-led-b", (int)rgb.b);
    }

    inline CRGB getCabinetInspectionLightColour() const {
        uint8_t r = (uint8_t)_getSetting("ins-led-r", 255);
        uint8_t g = (uint8_t) _getSetting("ins-led-g", 255);
        uint8_t b = (uint8_t) _getSetting("ins-led-b", 255);
        return CRGB(r, g, b);
    }

    inline void setInspectionTime(const TimeSpan& period) {
        _putSetting("ins-ms",  (int)period.millis());
    }

    inline TimeSpan getInspectionTime() const {
        return TimeSpan::fromMilliseconds(_getSetting("ins-ms", 10000));
    }

    inline void setCabinetLightBrightness(int brightness) {
        _putSetting("bri", (int)brightness);
    }

    inline int getCabinetLightBrightness() const {
        return _getSetting("bri", 64);
    }
};
