#pragma once

#include "Sensors/TemperatureSensor.h"
#include "Sensors/HumiditySensor.h"
#include "Sensors/TVOCSensor.h"
#include "Sensors/CO2Sensor.h"
#include "Sensors/HydrogenSensor.h"
#include "Sensors/EthenolSensor.h"
#include "Sensors/ParticleSensor.h"

#include "Sensors/TemperatureObserver.h"
#include "Sensors/HumidityObserver.h"
#include "Sensors/TVOCObserver.h"
#include "Sensors/CO2Observer.h"
#include "Sensors/HydrogenObserver.h"
#include "Sensors/EthenolObserver.h"
#include "Sensors/ParticleObserver.h"

#include <list>

class EnvironmentMonitor
{
public:

    EnvironmentMonitor();

    void attachTemperatureSensor(TemperatureSensor& sensor);
    void attachHumiditySensor(HumiditySensor& sensor);
    void attachTVOCSensor(TVOCSensor& sensor);
    void attachCO2Sensor(CO2Sensor& sensor);
    void attachHydrogenSensor(HydrogenSensor& sensor);
    void attachEthenolSensor(EthenolSensor& sensor);
    void attachParticleSensor(ParticleSensor& sensor);

    void addTemperatureObserver(TemperatureObserver& observer, bool reportInitial = false);
    void addHumidityObserver(HumidityObserver& observer, bool reportInitial = false);
    void addTVOCObserver(TVOCObserver& observer, bool reportInitial = false);
    void addCO2Observer(CO2Observer& observer, bool reportInitial = false);
    void addHydrogenObserver(HydrogenObserver& observer, bool reportInitial = false);
    void addEthenolObserver(EthenolObserver& observer, bool reportInitial = false);
    void addParticleObserver(ParticleObserver& observer, bool reportInitial = false);

    void setup();
    void loop();

private:

    typedef std::list<TemperatureObserver*> _TemperatureObservers;
    typedef std::list<HumidityObserver*> _HumidityObservers;
    typedef std::list<TVOCObserver*> _TVOCObservers;
    typedef std::list<CO2Observer*> _CO2Observers;
    typedef std::list<HydrogenObserver*> _HydrogenObservers;
    typedef std::list<EthenolObserver*> _EthenolObservers;
    typedef std::list<ParticleObserver*> _ParticleObservers;
    
    void _processTemperature();
    void _processHumidity();
    void _processTVOC();
    void _processCO2();
    void _processHydrogen();
    void _processEthenol();
    void _processParticles();

    void _notifyOfTemperature(float temperature);
    void _notifyOfHumidity(float humidity);
    void _notifyOfTVOC(uint16_t ugPerM3);
    void _notifyOfCO2(uint16_t ugPerM3);
    void _notifyOfHydrogen(uint16_t ppm);
    void _notifyOfEthenol(uint16_t ppm);
    void _notifyOfParticles(uint16_t pm01, uint16_t pm25, uint16_t pm10);

    TemperatureSensor* _temperatureSensor;
    HumiditySensor* _humiditySensor;
    TVOCSensor* _tvocSensor;
    CO2Sensor* _co2Sensor;
    HydrogenSensor* _hydrogenSensor;
    EthenolSensor* _ethenolSensor;
    ParticleSensor* _particleSensor;

    _TemperatureObservers _temperatureObservers;
    _HumidityObservers _humidityObservers;
    _TVOCObservers _tvocObservers;
    _CO2Observers _co2Observers;
    _HydrogenObservers _hydrogenObservers;
    _EthenolObservers _ethenolObservers;
    _ParticleObservers _particleObservers;

    float _temperature, _humidity;
    uint16_t _tvoc, _co2, _hydrogen, _ethenol;
    uint16_t _pm01, _pm25, _pm10;
};

