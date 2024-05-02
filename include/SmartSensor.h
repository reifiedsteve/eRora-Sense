#pragma once

#if 1

#include <list>

//#include "Sensors/TemperatureSensor.h"
#include "Sensors/TemperatureObserver.h"

//#include "Sensors/HumiditySensor.h"
#include "Sensors/HumidityObserver.h"

//#include "Sensors/TVOCSensor.h"
#include "Sensors/TVOCObserver.h"

//#include "Sensors/CO2Sensor.h"
#include "Sensors/CO2Observer.h"

//#include "Sensors/ParticleSensor.h"
#include "Sensors/ParticleObserver.h"

#include "Sensors/BME680Sensor.h"
#include "Sensors/SGP30Sensor.h"

// #include "PWMFanObserver.h"

#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

#include "Diagnostics/Logging.h"

/*
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
*/

#define BME_DEFAULT_I2C_ADDRESS 77

#define SEALEVELPRESSURE_HPA (1013.25)

class SmartSensor
{
public:

    class Observer
      : public TemperatureObserver
      , public HumidityObserver
      , public TVOCObserver
      , public CO2Observer
      // , public ParticleObserver
      // , public PWMFanObserver
    {
        virtual const char* name() = 0;
    };

    static const TimeSpan DefaultInterval; // = TimeSpan(1, TimeSpan::Units::Seconds);

    explicit SmartSensor()
      : _bmeSensor()
      , _sgpSensor()
      , _temperature(0)
      , _relHumidity(0)
    {}
    

    void registerObserver(Observer* observer) {
        _observers.push_back(observer);
    }


    void setup()
    {
        _bmeSensor.setup();
        Log.infoln("BME680 initialised.");

        _sgpSensor.setup();
        Log.infoln("SGP30 initialised.");

        _timer.restart();
    }

    void loop()
    {
        _bmeSensor.loop();

        float temperature(_bmeSensor.readTemperature());
        float relHumidity(_bmeSensor.readHumidity());

        // Apply calibration offsets.
        temperature = temperature - 4.0;
        relHumidity = relHumidity + 6.0;

        if (temperature != _temperature) {  // TODO: add a tolerance.
            _processTemperature(temperature);
            _temperature = temperature;
        }

        if (relHumidity != _relHumidity) {  // TODO: add a tolerance.
            _processHumidity(relHumidity);
            _relHumidity = relHumidity;
        }
    }

private:

    void _processTemperature(float temperature) {
        if (_temperature != temperature) {
            _temperature = temperature;
            //_informOfTemperature(_temperature);
        }
    }

    void _processHumidity(float relHumidity) {        
        if (_relHumidity != relHumidity) {
            _relHumidity = relHumidity;
            //_informOfHumidity(_sensor.readHumidity());
        }
    }

    typedef std::list<Observer*> _Observers;

    BME680Sensor _bmeSensor;
    SGP30Sensor _sgpSensor;

    float _temperature, _relHumidity;

    CountdownTimer _timer;

    _Observers _observers;
};

#endif
