#pragma once

#if 0

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
        if (_init()) {
            _bmeSensor.setup();
            Log.infoln("BME680 initialised.");
            _sgpSensor.setup();
            Log.infoln("SGP30 initialised.");
        }

        else {
            Log.errorln("BME680 failed to initialise - not connected?");
        }

        _timer.restart();
    }

    void loop()
    {
        float temperature(_bmeSensor.readTemperature());
        float relHumidity(_bmeSensor.readHumidity());

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

    bool _init()
    {
        bool ok(_sensor.begin(_i2cAddress));

        if (ok) {
            // Set up oversampling and filter initialization
            _sensor.setTemperatureOversampling(BME680_OS_8X);
            _sensor.setHumidityOversampling(BME680_OS_2X);
            _sensor.setPressureOversampling(BME680_OS_4X);
            _sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);
            _sensor.setGasHeater(320, 150); // 320*C for 150 ms        
        }

        return ok;    
    }

    void _processTemperature(float tempC) {
        if (_tempC != tempC) {
            _tempC = tempC;
            _informOfTemperature(_tempC);
        }
    }

    void _processHumidity(float humidity) {        
        if (_humidity != humidity) {
            _humidity = humidity;
            _informOfHumidity(_sensor.readHumidity());
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
