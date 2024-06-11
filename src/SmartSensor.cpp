#include <cmath>

#include "SmartSensor.h"
#include "PinAssignments.h"

#include "Diagnostics/Logging.h"

/*
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
*/

// #define BME_DEFAULT_I2C_ADDRESS 77

// #define SEALEVELPRESSURE_HPA (1013.25)

SmartSensor::SmartSensor()
    : _bmeSensor()
    #ifdef SMART_SENSOR_USES_PMS7003
    , _pmSensor(
        PinAssignments::PMSerialRX,
        PinAssignments::PMSerialTX,
        TimeSpan::fromMilliseconds(100)
    )
    #endif
    , _state()
    , _timer(5000)
    , _observers()
    , _fan(nullptr)
    , _fanSpeed(1)
{}

void SmartSensor::bindObserver(Observer& observer)
{
    _ScopedLock lock(_mutex);
    _observers.push_back(&observer);

    // TODO: Ooops. This will be too many messages for web WS to handle!
    // Observer needs a method to take all values in one hit.
    
    observer.onSwitchOnOff(_state.power);
    observer.onFanSpeed(_state.fanSpeed);
    observer.onTemperature(_state.temperature);
    observer.onAirPressure(_state.airPressure);
    observer.onHumidity(_state.relHumidity);
    observer.onAirPressure(_state.airPressure);
    observer.onIAQAvailability(_state.sensorReady);
    observer.onIAQ(_state.iaq);
    observer.onTVOC(_state.tvoc);
    observer.onCO2(_state.co2);
    #ifdef SMART_SENSOR_USES_PMS7003
    observer.onPM01(_pm01);
    observer.onPM25(_pm25);
    observer.onPM10(_pm10);
    #endif
}

void SmartSensor::bindFanController(FanController& controller) {
    _ScopedLock lock(_mutex);
    _fan = &controller;
    _fan->setPower(_state.power);
    _fan->setSpeed(10 * _state.fanSpeed); // As underlying speed control is a percentage.
}

void SmartSensor::switchPower(bool on) {
    _ScopedLock lock(_mutex);
    _setPower(on);
}

void SmartSensor::togglePower() {
    _ScopedLock lock(_mutex);
    _setPower(!_state.power);
}

void SmartSensor::selectNextMode() {
    _ScopedLock lock(_mutex);
    // TODO
}

void SmartSensor::setFanSpeed(int speed) {
    _ScopedLock lock(_mutex);
    Log.infoln("SmartSensor: setting fan speed to %d.", speed);
    _doSetFanSpeed(speed);
    Log.infoln("SmartSensor: fan speed is now %d.", _state.fanSpeed);
}

void SmartSensor::adjustFanSpeed(int delta) {
    _ScopedLock lock(_mutex);
    Log.infoln("SmartSensor: adjusting fan speed by %d.", delta);
    _doAdjustFanSpeed(delta);
    Log.infoln("SmartSensor: fan speed is now %d.", _state.fanSpeed);
}

void SmartSensor::reboot()
{
    _ScopedLock lock(_mutex);
    Log.infoln("SmartSensor: reboot.");

    _setPower(false);

    // TODO: send a "rebooting" notification to all observers to give 
    // them a chance to reflect that to the user.
    // TODO: Do we want to also clear down the preview LEDs? 

    ESP.restart();
}

bool SmartSensor::isOn() const {
    _ScopedLock lock(_mutex);
    return _state.power;
}

int SmartSensor::getFanSpeed() const {
    _ScopedLock lock(_mutex);
    return _state.fanSpeed;
}

void SmartSensor::setup()
{
    _bmeSensor.setup();
    Log.infoln("BME680 initialised.");

    #ifdef SMART_SENSOR_USES_PMS7003
    _pmSensor.setup();
    Log.infoln("PM7003 initialised.");
    #endif

    //_sgpSensor.setup();
    //Log.infoln("SGP30 initialised.");

    _timer.start();
}

void SmartSensor::loop()
{
    // TODO: Get calibrated offsets from user settings.
    _bmeSensor.setTemperatureOffset(-2.0);
    _bmeSensor.setHumidityOffset(+6.0);

    _bmeSensor.loop();

    #ifdef SMART_SENSOR_USES_PMS7003
    _pmSensor.loop();
    #endif

    if (_timer.hasExpired()) 
    {
        // Log.verboseln("Processing...");
        _timer.restart();

        bool gotSomething(false);

        if (_bmeSensor.isTemperatureAvailable()) {
            // Log.verboseln("Temperature available...");
            gotSomething = true;
            _processTemperature(_bmeSensor.readTemperature());
        }

        if (_bmeSensor.isHumidityAvailable()) {
            // Log.verboseln("Humidity available...");
            gotSomething = true;
            _processHumidity(_bmeSensor.readHumidity());
        }

        if (_bmeSensor.isAirPressureAvailable()) {
            // Log.verboseln("Air pressure available...");
            gotSomething = true;
            _processAirPressure(_bmeSensor.readAirPressure());
        }

        if (_bmeSensor.isTVOCAvailable()) {
            // Log.verboseln("TVOC available...");
            gotSomething = true;
            _processTVOC(_bmeSensor.readTVOC());
        }

        if (_bmeSensor.isCO2Available()) {
            // Log.verboseln("CO2 available...");
            gotSomething = true;
            _processCO2(_bmeSensor.readCO2());
        }

        if (_bmeSensor.isIAQAvailable()) {
            // Log.verboseln("IAQ available...");
            gotSomething = true;
            _processIAQ(_bmeSensor.isIAQAvailable(), _bmeSensor.readIAQ());
        }

        if (!gotSomething) {
            Log.verboseln("No BME680 readings available.");
        }

        #ifdef SMART_SENSOR_USES_PMS7003

        if (_pmSensor.available())
        {
            PMS7003ParticleSensor::Measurements measurements(_pmSensor.read());

            _processPM01(measurements.pm01);
            _processPM25(measurements.pm25);
            _processPM10(measurements.pm10);
        }

        else {
            Log.verboseln("No PMS7003 readings available.");
        }

        #endif
    }
}

void SmartSensor::_doSetFanSpeed(int speed) {
    _setFanSpeed(_constrainFanSpeed(speed));
}

void SmartSensor::_doAdjustFanSpeed(int delta) {
    int newSpeed(_fanSpeed + delta);
    _doSetFanSpeed(newSpeed);
}

bool SmartSensor::_setPower(bool on) {
    bool changed(on != _state.power);
    if (changed && _fan) {
        Log.verboseln("SmartSensor: fan power now %s.", on ? "on" : "off");
        _fan->setPower(on);
        _state.power = on;
        _informOfPower(on);
    }
    return changed;
}

bool SmartSensor::_setFanSpeed(int newFanSpeed)
{
    int oldFanSpeed(_state.fanSpeed);

    // bool changed(newFanSpeed != oldFanSpeed);
    bool changed(true);

    if (changed && (_fan != nullptr))
    {
        Log.verboseln("SmartSensor: fan speed changed from %d to %d.", oldFanSpeed, newFanSpeed);

        #if 0

        // We interpret setting a speed of zero
        // to mean turn off the fan.

        if (newFanSpeed == 0) {
            _fan->setPower(false);
        }

        else if (oldFanSpeed == 0) {
            _fan->setPower(true);
        }

        #endif

        _fan->setSpeed((uint8_t)(10 * newFanSpeed));

        _state.fanSpeed = newFanSpeed;
        _informOfFanSpeed(newFanSpeed);
    }

    return changed;
}

void SmartSensor::_processTemperature(float temperature) {
    if (_state.temperature != temperature) {
        _state.temperature = temperature;
        _informOfTemperature(temperature);
    }
}

void SmartSensor::_processHumidity(float relHumidity) {        
    if (_state.relHumidity != relHumidity) {
        _state.relHumidity = relHumidity;
        _informOfHumidity(relHumidity);
    }
}

void SmartSensor::_processAirPressure(float airPressure) {        
    if (_state.airPressure != airPressure) {
        _state.airPressure = airPressure;
        _informOfAirPressure(airPressure);
    }
}

void SmartSensor::_processTVOC(float tvoc) {
    if (_state.tvoc != tvoc) {
        _state.tvoc = tvoc;
        _informOfTVOC(tvoc);
    }
}

void SmartSensor::_processCO2(float co2) {
    if (_state.co2 != co2) {
        _state.co2 = co2;
        _informOfCO2(co2);
    }
}

void SmartSensor::_processIAQ(bool available, float iaq)
{
    if (_state.sensorReady |= available) {
        _informOfIAQAvailability(available);
        _state.sensorReady = available;
    }

    if (_state.sensorReady) {
        if (_state.iaq != iaq) {
            _state.iaq = iaq;
            _informOfIAQ(iaq);
        }
    }
}

void SmartSensor::_processPM01(uint16_t pm01) {
    _informOfPM01(pm01);
}

void SmartSensor::_processPM25(uint16_t pm25) {
    _informOfPM25(pm25);
}

void SmartSensor::_processPM10(uint16_t pm10) {
    _informOfPM10(pm10);
}

void SmartSensor::_informOfTemperature(float temperature) {
    for (Observer* observer: _observers) {
        // Log.verboseln("Informing \"%s\" about temperature of %d.", observer->observerName(), (int) temperature);
        observer->onTemperature(temperature);
    }
}

void SmartSensor::_informOfHumidity(float humidity) {
    for (Observer* observer: _observers) {
        // Log.verboseln("Informing \"%s\" about humidity of %d.", observer->observerName(), (int) humidity);
        observer->onHumidity(humidity);
    }
}

void SmartSensor::_informOfAirPressure(float airPressure) {
    for (Observer* observer: _observers) {
        // Log.verboseln("Informing \"%s\" about air pressure of of %d hPa.", observer->observerName(), (int) airPressure);
        observer->onAirPressure(airPressure);
    }
}

void SmartSensor::_informOfTVOC(float tvoc) {
    for (Observer* observer: _observers) {
        // Log.verboseln("Informing \"%s\" about TVOC of %d.", observer->observerName(), (int) tvoc);
        observer->onTVOC(tvoc);
    }
}

void SmartSensor::_informOfCO2(float co2) {
    for (Observer* observer: _observers) {
        // Log.verboseln("Informing \"%s\" about CO2 of %d.", observer->observerName(), (int) co2);
        observer->onCO2(co2);
    }
}

void SmartSensor::_informOfIAQ(float iaq) {
    for (Observer* observer: _observers) {
        // Log.verboseln("Informing \"%s\" about IAQ of %d.", observer->observerName(), (int) iaq);
        observer->onIAQ(iaq);
    }
}

void SmartSensor::_informOfPM01(uint16_t pm01) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about PM 1.0 of %d ug/m3.", observer->observerName(), (int) pm01);
        observer->onPM01(pm01);
    }
}

void SmartSensor::_informOfPM25(uint16_t pm25) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about PM 2.5 of %d ug/m3.", observer->observerName(), (int) pm25);
        observer->onPM25(pm25);
    }
}

void SmartSensor::_informOfPM10(uint16_t pm10) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about PM 10.0 of %d ug/m3.", observer->observerName(), (int) pm10);
        observer->onPM10(pm10);
    }
}

void SmartSensor::_informOfIAQAvailability(bool available) {
    for (Observer* observer: _observers) {
        // Log.verboseln("Informing \"%s\" about IAQ availability of %s.", observer->observerName(), available ? "yes" : "no");
        observer->onIAQAvailability(available);
    }
}

void SmartSensor::_informOfPower(bool on) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about power %s.", observer->observerName(), on ? "on" : "off");
        observer->onSwitchOnOff(on);
    }
}

void SmartSensor::_informOfFanSpeed(int fanSpeed) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about fan speed of %d.", observer->observerName(), fanSpeed);
        observer->onFanSpeed(fanSpeed);
    }
}

int SmartSensor::_constrainFanSpeed(int speed) {
    return std::max(MinFanSpeed, std::min(speed, MaxFanSpeed));
}

const TimeSpan SmartSensor::DefaultInterval(5, TimeSpan::Units::Seconds);


