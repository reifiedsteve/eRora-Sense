#pragma once

#include "TemperatureObserver.h"
#include "HumidityObserver.h"
#include "TVOCObserver.h"
#include "CO2Observer.h"
#include "ParticleObserver.h"

#include "Diagnostics/Logging.h"

#include <sstream>

class EnvironmentLogger
  : public TemperatureObserver
  , public HumidityObserver
  , public TVOCObserver
  , public CO2Observer
  , public ParticleObserver
{
public:

    void onTemperature(float temperature) override {
        Log.infoln("Temperature: %s°C", _stringify(temperature).c_str());        
    }

    void onHumidity(float humidity) override {
        // Log.infoln("Humidity: %s%%", _stringify(humidity).c_str());        
        Log.infoln("Humidity: %d%%", (int)(humidity));        
    }

    void onTVOC(uint16_t airPressure) override {
        // Log.infoln("Air Pressure: %s hPa", _stringify(airPressure).c_str());        
        Log.infoln("TVOC: %d ug/m3", (int)(airPressure));        
    }

    void onCO2(uint16_t co2) override {
        Log.infoln("CO2: %d ug/m3", co2);        
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
        ss << (state ? "Yes" : "No");
        return ss.str();
    }
};
