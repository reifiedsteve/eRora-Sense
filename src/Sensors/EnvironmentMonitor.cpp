
#include "Sensors/EnvironmentMonitor.h"

EnvironmentMonitor::EnvironmentMonitor() 
    : _temperatureSensor(nullptr)
    , _humiditySensor(nullptr)
    , _airPressureSensor(nullptr)
    , _gasLevelSensor(nullptr)
    , _occupancySensor(nullptr)
    , _temperatureObservers()
    , _humidityObservers()
    , _airPressureObservers()
    , _gasLevelObservers()
    , _occupancyObservers()
{}

void EnvironmentMonitor::attachTemperatureSensor(TemperatureSensor& sensor) {
    _temperatureSensor = &sensor;
}

void EnvironmentMonitor::attachHumiditySensor(HumiditySensor& sensor) {
    _humiditySensor = &sensor;
}

void EnvironmentMonitor::attachAirPressureSensor(AirPressureSensor& sensor) {
    _airPressureSensor = &sensor;
}

void EnvironmentMonitor::attachGasLevelSensor(GasLevelSensor& sensor) {
    _gasLevelSensor = &sensor;
}

void EnvironmentMonitor::attachOccupancySensor(OccupancySensor& sensor) {
    _occupancySensor = &sensor;
}

void EnvironmentMonitor::addTemperatureObserver(TemperatureObserver& observer, bool reportInitial) {
    _temperatureObservers.push_back(&observer);
    if (reportInitial) observer.onTemperature(_temperature);
}

void EnvironmentMonitor::addHumidityObserver(HumidityObserver& observer, bool reportInitial) {
    _humidityObservers.push_back(&observer);
    if (reportInitial) observer.onHumidity(_humidity);
}

void EnvironmentMonitor::addAirPressureObserver(AirPressureObserver& observer, bool reportInitial) {
    _airPressureObservers.push_back(&observer);
    if (reportInitial) observer.onAirPressure(_airPressure);
}

void EnvironmentMonitor::addGasLevelObserver(GasLevelObserver& observer, bool reportInitial) {
    _gasLevelObservers.push_back(&observer);
    if (reportInitial) observer.onGasLevel(_gasLevel);
}

void EnvironmentMonitor::addOccupancyObserver(OccupancyObserver& observer, bool reportInitial) {
    _occupancyObservers.push_back(&observer);
    if (reportInitial) observer.onOccupancy(_occupied);
}

void EnvironmentMonitor::setup() {
    // ?
}

void EnvironmentMonitor::loop()
{
    _processTemperature();
    _processHumidity();
    _processAirPressure();
    _processGasLevel();
    _processOccupancy();

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

void EnvironmentMonitor::_processAirPressure() {
    if (_airPressureSensor /* && _airPressureSensor->isAirPressureAvailable() */) {
        float airPressure(_airPressureSensor->readAirPressure());
        if (_airPressure != airPressure) { // ??? Or use a defineable tolerance?
            _airPressure = airPressure;
            _notifyOfAirPressure(airPressure);
        }
    }
}

void EnvironmentMonitor::_processGasLevel() {
    if (_gasLevelSensor /* && _gasLevelSensor->isGasLevelAvailable() */) {
        float gasLevel(_gasLevelSensor->readGasLevel());
        if (_gasLevel != gasLevel) { // ??? Or use a defineable tolerance?
            _gasLevel = gasLevel;
            _notifyOfGasLevel(gasLevel);
        }
    }
}

void EnvironmentMonitor::_processOccupancy() {
    if (_occupancySensor /* && _occupancySensor->isOccupancyAvailable() */ ) {
        bool occupied(_occupancySensor->readOccupancy());
        if (_occupied != occupied) { // ??? Or use a defineable tolerance?
            _occupied = occupied;
            _notifyOfOccupancy(occupied);
        }
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

void EnvironmentMonitor::_notifyOfAirPressure(float airPressure) {
    for (AirPressureObserver* observer : _airPressureObservers) {
        observer->onAirPressure(airPressure);
    }
}

void EnvironmentMonitor::_notifyOfGasLevel(float gasLevel) {
    for (GasLevelObserver* observer : _gasLevelObservers) {
        observer->onGasLevel(gasLevel);
    }
}

void EnvironmentMonitor::_notifyOfOccupancy(bool occupied) {
    for (OccupancyObserver* observer : _occupancyObservers) {
        observer->onOccupancy(occupied);
    }
}
