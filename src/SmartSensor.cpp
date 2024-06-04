#include <cmath>

#include "SmartSensor.h"

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
    // , _sgpSensor()
    , _temperature(0)
    , _relHumidity(0)
    , _co2(0)
    , _tvoc(0)
    , _iaqAvailable(false)
    , _iaq(0)
    , _timer(5000)
    , _observers()
    , _fanController(nullptr)
    , _fanSpeed(5)
{}


void SmartSensor::bindObserver(Observer& observer) {
    _observers.push_back(&observer);
}

void SmartSensor::bindFanController(FanController& controller) {
    _fanController = &controller;
    Log.verboseln("At (A)");
    _fanController->setFanSpeed(10 * _fanSpeed);
    Log.verboseln("At (B)");
}

void SmartSensor::switchPower(bool on) {

}

void SmartSensor::togglePower() {

}

void SmartSensor::selectNextMode() {

}

void SmartSensor::setFanSpeed(uint8_t speedSetting) {
    Log.infoln("SmartSensor: setting fan speed to %d.", speedSetting);
    _setFanSpeed(speedSetting);
    Log.infoln("SmartSensor: fan speed is now %d.", _fanSpeed);
}

void SmartSensor::adjustFanSpeed(int8_t delta) {
    Log.infoln("SmartSensor: adjusting fan speed by %d.", delta);
    _setFanSpeed(_fanSpeed + delta);
    Log.infoln("SmartSensor: fan speed is now %d.", _fanSpeed);
}

void SmartSensor::setup()
{
    _bmeSensor.setup();
    Log.infoln("BME680 initialised.");

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

    if (_timer.hasExpired()) 
    {
        Log.verboseln("Processing...");
        _timer.restart();

        bool gotSomething(false);

        if (_bmeSensor.isTemperatureAvailable()) {
            Log.verboseln("Temperature available...");
            gotSomething = true;
            _processTemperature(_bmeSensor.readTemperature());
        }

        if (_bmeSensor.isHumidityAvailable()) {
            Log.verboseln("Humidity available...");
            gotSomething = true;
            _processHumidity(_bmeSensor.readHumidity());
        }

        if (_bmeSensor.isTVOCAvailable()) {
            Log.verboseln("TVOC available...");
            gotSomething = true;
            _processTVOC(_bmeSensor.readTVOC());
        }

        if (_bmeSensor.isCO2Available()) {
            Log.verboseln("CO2 available...");
            gotSomething = true;
            _processCO2(_bmeSensor.readCO2());
        }

        if (_bmeSensor.isIAQAvailable()) {
            Log.verboseln("IAQ available...");
            gotSomething = true;
            _processIAQ(_bmeSensor.isIAQAvailable(), _bmeSensor.readIAQ());
        }

        if (!gotSomething) {
            Log.verboseln("No readiings available.");
        }
    }
}

bool SmartSensor::_setFanSpeed(uint8_t speedSetting) {
    uint8_t newFanSpeed = std::max((uint8_t)0, std::min(speedSetting, (uint8_t)10));
    bool changed(_fanSpeed != newFanSpeed);
    if (changed && _fanController) {
        Log.verboseln("SmartSensor: fan speed changed from %d to %d.", _fanSpeed, newFanSpeed);
        _fanController->setFanSpeed(10 * newFanSpeed);
        _fanSpeed = newFanSpeed;
        _informOfFanSpeed(_fanSpeed);
    }    
    return changed;
}

void SmartSensor::_processTemperature(float temperature) {
    if (_temperature != temperature) {
        _temperature = temperature;
        _informOfTemperature(temperature);
    }
}

void SmartSensor::_processHumidity(float relHumidity) {        
    if (_relHumidity != relHumidity) {
        _relHumidity = relHumidity;
        _informOfHumidity(relHumidity);
    }
}

void SmartSensor::_processTVOC(float tvoc) {
    if (_tvoc != tvoc) {
        _tvoc = tvoc;
        _informOfTVOC(_tvoc);
    }
}

void SmartSensor::_processCO2(float co2) {
    if (_co2 != co2) {
        _co2 = co2;
        _informOfCO2(_co2);
    }
}

void SmartSensor::_processIAQ(bool available, float iaq) {
    if (_iaqAvailable |= available) {
        _informOfIAQAvailability(available);
        _iaqAvailable = available;
    }
    if (_iaqAvailable) {
        if (_iaq != iaq) {
            _iaq = iaq;
            _informOfIAQ(iaq);
        }
    }
}

void SmartSensor::_informOfTemperature(float temperature) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about temperature of %d.", observer->observerName(), (int) temperature);
        observer->onTemperature(temperature);
    }
}

void SmartSensor::_informOfHumidity(float humidity) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about humidity of %d.", observer->observerName(), (int) humidity);
        observer->onHumidity(humidity);
    }
}

void SmartSensor::_informOfTVOC(float tvoc) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about TVOC of %d.", observer->observerName(), (int) tvoc);
        observer->onTVOC(tvoc);
    }
}

void SmartSensor::_informOfCO2(float co2) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about CO2 of %d.", observer->observerName(), (int) co2);
        observer->onCO2(co2);
    }
}

void SmartSensor::_informOfIAQ(float iaq) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about IAQ of %d.", observer->observerName(), (int) iaq);
        observer->onIAQ(iaq);
    }
}

void SmartSensor::_informOfIAQAvailability(bool available) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about IAQ availability of %s.", observer->observerName(), available ? "yes" : "no");
        observer->onIAQAvailability(available);
    }
}

void SmartSensor::_informOfFanSpeed(uint8_t fanSpeed) {
    for (Observer* observer: _observers) {
        Log.verboseln("Informing \"%s\" about fan speed of %d.", observer->observerName(), fanSpeed);
        observer->onFanSpeed(fanSpeed);
    }
}

const TimeSpan SmartSensor::DefaultInterval(5, TimeSpan::Units::Seconds);


