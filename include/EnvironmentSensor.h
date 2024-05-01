#pragma once

#if 0

#include "Diagnostics/Logging.h"

//#include <Wire.h>
//#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Arduino.h>
#include <PMserial.h>

// Sensor types to support (more widely):
//    temperature
//    humidity
//    VoC / IAQ.
//    luminance.
//    human presense.
//    CO2 levels?
// Note: LD2410 devices can hit 48C, so quite hot! 
// Hence need isolating from any temo sensor.

#include <list>
#include <sstream>

#if 0

enum class InitialReportMode {
    No,
    Yes,
    IfSet
};

template <typename T>
class ObservedValue
{
public:

    typedef std::function<void(T value)> Observer;

    ObservedValue(T initialValue, T changeDelta)
      : _value(initialValue)
      , _beenSet(false)
      , _observers()
    {}

    void registerObserver(Observer observer, InitialReportMode reportMode) {
        _observers.push_back(observer);
        if ((reportMode == InitialReportMode::Yes) || ((reportMode == InitialReportMode::IfSet) && _beenSet)) {
            observer(_reportedValue);
        }
    }

    /*
    inline void setValue(T newValue) {
        _setValue(newValue);
    }
    */

    ObservedValue& operator=(const T& value) {
        _setValue(value);
        return *this;
    }

    ObservedValue& operator=(const ObservedValue& rhs) {
        _setvalue(rhs._value);
        return *this;
    }

    operator T() const { 
        return _value;
    }

private:

    typedef std::list<Observer> _Observers;

    void _setValue(T newValue) {
        _beenSet = true;
        // Avoid std::abs due to possible unsigned T.
        if (newValue != _value) {
            _value = newValue;
            _notifyObservers(_value);
        }
    }

    void _notifyObservers(T value) {
        for (Observer observer: _observers) {
            observer(value);
        }
    }

    T _value;
    bool _beenSet;

    _Observers _observers;
};

#endif

#if 1

#include "Environment/TemperatureSensor.h"
#include "Environment/HumiditySensor.h"
#include "Environment/AirPressureSensor.h"
#include "Environment/GasLevelSensor.h"
#include "Environment/OccupancySensor.h"

#endif

#include "Environment/EnvironmentMonitor.h"


#endif
