#pragma once

#include "SmartSensorObserver.h"

#include "Diagnostics/Logging.h"

#include <sstream>

class EnvironmentLogger
  : public SmartSensorObserver
{
public:

    EnvironmentLogger() : SmartSensorObserver("env-logger") {}

    void onSwitchOnOff(bool on) override {
        Log.infoln("Power: %s", on ? "on" : "off");        
    }

    void onFanSpeed(int speed) override {
        Log.infoln("Fan: %d", speed);
    }

    void onBacklightBrightness(uint8_t brightness) override {
        Log.infoln("Brightness: %d", (int)brightness);
    }

    void onBacklightColour(uint8_t hue, uint8_t sat) override {
        Log.infoln("Hue: %d, Sat: %d", (int)hue, (int)sat);
    }

    // virtual void onDisplayMode(DisplayMode mode) = 0;

    void onTemperature(float temperature) override {
        Log.infoln("Temperature: %s°C", _stringify(temperature).c_str());        
    }

    void onHumidity(float humidity) override {
        // Log.infoln("Humidity: %s%%", _stringify(humidity).c_str());        
        Log.infoln("Humidity: %d%%", (int)(humidity));        
    }

    void onAirPressure(float hPa) override {
        Log.infoln("AIr pressure: %d hPa", (int)hPa / 100);
    }

    void onTVOC(float airPressure) override {
        // Log.infoln("Air Pressure: %s hPa", _stringify(airPressure).c_str());        
        Log.infoln("TVOC: %d ppb", (int)(airPressure));        
    }

    void onCO2(float co2) override {
        Log.infoln("CO2: %d ug/m3", (int)co2);
    }

    void onIAQ(float iaq) override {
        Log.infoln("IAQ: %d", (int)iaq);
    }
#
    void onIAQAvailability(bool available) {
        Log.infoln("IAQ Available: %s", available ? "yes" : "no");
    }

    void onParticleReading(uint16_t pm01, uint16_t pm25, uint16_t pm10) {
        // Log.infoln("Particles: PM1.0 %d, PM2.5 %d, PM10.0 %d", pm01, pm25, pm10);
    }

private:

    static std::string _stringify(float val) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }

    static std::string _stringify(bool state) {
        std::ostringstream ss;
        ss << (state ? "yes" : "no");
        return ss.str();
    }
};
