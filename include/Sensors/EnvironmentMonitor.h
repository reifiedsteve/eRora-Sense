#pragma once

#include "Sensors/TemperatureSensor.h"
#include "Sensors/HumiditySensor.h"
#include "Sensors/AirPressureSensor.h"
#include "Sensors/GasLevelSensor.h"
#include "Sensors/OccupancySensor.h"

#include "Sensors/TemperatureObserver.h"
#include "Sensors/HumidityObserver.h"
#include "Sensors/AirPressureObserver.h"
#include "Sensors/GasLevelObserver.h"
#include "Sensors/OccupancyObserver.h"

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

