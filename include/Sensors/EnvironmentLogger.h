#pragma once

#include "TemperatureObserver.h"
#include "HumidityObserver.h"
#include "AIrPressureObserver.h"
#include "GasLevelObserver.h"
#include "OccupancyObserver.h"
#include "ParticleObserver.h"

#include "Diagnostics/Logging.h"

#include <sstream>

class EnvironmentLogger
  : public TemperatureObserver
  , public HumidityObserver
  , public AirPressureObserver
  , public GasLevelObserver
  , public OccupancyObserver
  , public ParticleObserver
{
public:

    void onTemperature(float temperature) override {
        Log.infoln("Temperature: %sC", _stringify(temperature).c_str());        
    }

    void onHumidity(float humidity) override {
        Log.infoln("Humidity: %s%%", _stringify(humidity).c_str());        
    }

    void onAirPressure(float airPressure) override {
        Log.infoln("Air Pressure: %shPa", _stringify(airPressure).c_str());        
    }

    void onGasLevel(float gasLevel) override {
        Log.infoln("Gas Level: %sKOhms", _stringify(gasLevel).c_str());        
    }

    void onOccupancy(bool occupied) override {
        Log.infoln("Occupancy: %s", _stringify(occupied).c_str());        
    }

    void onParticleReading(uint16_t pm01, uint16_t pm25, uint16_t pm10) {
        Log.infoln("Particles: PM1.0 %d, PM2.5 %d, PM10.0 %d", pm01, pm25, pm10);
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
