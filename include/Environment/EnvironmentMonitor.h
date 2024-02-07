#pragma once

#include "Environment/TemperatureSensor.h"
#include "Environment/HumiditySensor.h"
#include "Environment/AirPressureSensor.h"
#include "Environment/GasLevelSensor.h"
#include "Environment/OccupancySensor.h"

#include "Environment/TemperatureObserver.h"
#include "Environment/HumidityObserver.h"
#include "Environment/AirPressureObserver.h"
#include "Environment/GasLevelObserver.h"
#include "Environment/OccupancyObserver.h"

#include <list>

class EnvironmentMonitor
{
public:

    EnvironmentMonitor();

    void attachTemperatureSensor(TemperatureSensor& sensor);
    void attachHumiditySensor(HumiditySensor& sensor);
    void attachAirPressureSensor(AirPressureSensor& sensor);
    void attachGasLevelSensor(GasLevelSensor& sensor);
    void attachOccupancySensor(OccupancySensor& sensor);

    void addTemperatureObserver(TemperatureObserver& observer, bool reportInitial = false);
    void addHumidityObserver(HumidityObserver& observer, bool reportInitial = false);
    void addAirPressureObserver(AirPressureObserver& observer, bool reportInitial = false);
    void addGasLevelObserver(GasLevelObserver& observer, bool reportInitial = false);
    void addOccupancyObserver(OccupancyObserver& observer, bool reportInitial = false);

    void setup();
    void loop();

private:

    typedef std::list<TemperatureObserver*> _TemperatureObservers;
    typedef std::list<HumidityObserver*> _HumidityObservers;
    typedef std::list<AirPressureObserver*> _AirPressureObservers;
    typedef std::list<GasLevelObserver*> _GasLevelObservers;
    typedef std::list<OccupancyObserver*> _OccupancyObservers;
    
    void _processTemperature();
    void _processHumidity();
    void _processAirPressure();
    void _processGasLevel();
    void _processOccupancy();

    void _notifyOfTemperature(float temperature);
    void _notifyOfHumidity(float humidity);
    void _notifyOfAirPressure(float airPressure);
    void _notifyOfGasLevel(float gasLevel);
    void _notifyOfOccupancy(bool occupied);

    TemperatureSensor* _temperatureSensor;
    HumiditySensor* _humiditySensor;
    AirPressureSensor* _airPressureSensor;
    GasLevelSensor* _gasLevelSensor;
    OccupancySensor* _occupancySensor;

    _TemperatureObservers _temperatureObservers;
    _HumidityObservers _humidityObservers;
    _AirPressureObservers _airPressureObservers;
    _GasLevelObservers _gasLevelObservers;
    _OccupancyObservers _occupancyObservers;
    
    float _temperature, _humidity, _airPressure, _gasLevel;
    bool _occupied;
};

