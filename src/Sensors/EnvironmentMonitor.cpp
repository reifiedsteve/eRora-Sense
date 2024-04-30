
#include "Sensors/EnvironmentMonitor.h"

EnvironmentMonitor::EnvironmentMonitor() 
    : _temperatureSensor(nullptr)
    , _humiditySensor(nullptr)
    , _tvocSensor(nullptr)
    , _co2Sensor(nullptr)
    , _particleSensor(nullptr)
    , _temperatureObservers()
    , _humidityObservers()
    , _tvocObservers()
    , _co2Observers()
    , _particleObservers()
    , _temperature(0.0)
    , _humidity(0.0)
    , _tvoc(0)
    , _co2(0)
    , _pm01(0)
    , _pm25(0)
    , _pm10(0)
{}

void EnvironmentMonitor::attachTemperatureSensor(TemperatureSensor& sensor) {
    _temperatureSensor = &sensor;
}

void EnvironmentMonitor::attachHumiditySensor(HumiditySensor& sensor) {
    _humiditySensor = &sensor;
}

void EnvironmentMonitor::attachTVOCSensor(TVOCSensor& sensor) {
    _tvocSensor = &sensor;
}

void EnvironmentMonitor::attachCO2Sensor(CO2Sensor& sensor) {
    _co2Sensor = &sensor;
}

void EnvironmentMonitor::attachParticleSensor(ParticleSensor& sensor) {
    _particleSensor = &sensor;
}

void EnvironmentMonitor::addTemperatureObserver(TemperatureObserver& observer, bool reportInitial) {
    _temperatureObservers.push_back(&observer);
    if (reportInitial) observer.onTemperature(_temperature);
}

void EnvironmentMonitor::addHumidityObserver(HumidityObserver& observer, bool reportInitial) {
    _humidityObservers.push_back(&observer);
    if (reportInitial) observer.onHumidity(_humidity);
}

void EnvironmentMonitor::addTVOCObserver(TVOCObserver& observer, bool reportInitial) {
    _tvocObservers.push_back(&observer);
    if (reportInitial) observer.onTVOC(_tvoc);
}

void EnvironmentMonitor::addCO2Observer(CO2Observer& observer, bool reportInitial) {
    _co2Observers.push_back(&observer);
    if (reportInitial) observer.onCO2(_co2);
}

void EnvironmentMonitor::addParticleObserver(ParticleObserver& observer, bool reportInitial) {
    _particleObservers.push_back(&observer);
    if (reportInitial) observer.onParticleReading(_pm01, _pm25, _pm10);
}

void EnvironmentMonitor::setup() {
    // ?
}

void EnvironmentMonitor::loop()
{
    _processTemperature();
    _processHumidity();
    _processTVOC();
    _processCO2();
    _processParticles();

    // TODO: etc.
}

void EnvironmentMonitor::_processTemperature() {
    if (_temperatureSensor /* && _temperatureSensor->isTemperatureAvailable() */ ) {
        float temperature(_temperatureSensor->readTemperature());
        if (_temperature != temperature) { // ??? Or use a defineable tolerance?
            _temperature = temperature;
            _notifyOfTemperature(temperature);
        }
    }
}

void EnvironmentMonitor::_processHumidity() {
    if (_humiditySensor /* && _humiditySensor->isHumidityAvailable() */ ) {
        float humidity(_humiditySensor->readHumidity());
        if (_humidity != humidity) { // ??? Or use a defineable tolerance?
            _humidity = humidity;
            _notifyOfHumidity(humidity);
        }
    }
}

void EnvironmentMonitor::_processTVOC() {
    if (_tvocSensor /* && _tvocSensor->isAirPressureAvailable() */) {
        uint16_t tvoc(_tvocSensor->readTVOC());
        if (_tvoc != tvoc) { // ??? Or use a defineable tolerance?
            _tvoc = tvoc;
            _notifyOfTVOC(tvoc);
        }
    }
}

void EnvironmentMonitor::_processCO2() {
    if (_co2Sensor /* && _co2Sensor->isGasLevelAvailable() */) {
        float co2(_co2Sensor->readCO2());
        if (_co2 != co2) { // ??? Or use a defineable tolerance?
            _co2 = co2;
            _notifyOfCO2(co2);
        }
    }
}

void EnvironmentMonitor::_processParticles() {
    if (_particleSensor  /* && _particleSensor->available() */ ) {
        ParticleSensor::Measurements levels(_particleSensor->read());
        //if ((levels.pm01 != _pm01) || (levels.pm25 != _pm25) || (levels.pm10 != _pm10)) {
            _pm01 = levels.pm01;
            _pm25 = levels.pm25;
            _pm10 = levels.pm10;
            _notifyOfParticles(_pm01, _pm25, _pm10);
        //}
    }
}

void EnvironmentMonitor::_notifyOfTemperature(float temperature) {
    for (TemperatureObserver* observer : _temperatureObservers) {
        observer->onTemperature(temperature);
    }
}

void EnvironmentMonitor::_notifyOfHumidity(float humidity) {
    for (HumidityObserver* observer : _humidityObservers) {
        observer->onHumidity(humidity);
    }
}

void EnvironmentMonitor::_notifyOfTVOC(uint16_t tvoc) {
    for (TVOCObserver* observer : _tvocObservers) {
        observer->onTVOC(tvoc);
    }
}

void EnvironmentMonitor::_notifyOfCO2(uint16_t co2) {
    for (CO2Observer* observer : _co2Observers) {
        observer->onCO2(co2);
    }
}

void EnvironmentMonitor::_notifyOfParticles(uint16_t pm01, uint16_t pm25, uint16_t pm10) {
    for (ParticleObserver* observer : _particleObservers) {
        observer->onParticleReading(pm01, pm25, pm10);
    }
}
