#pragma once

#if 1

#include <list>

#include "Sensors/TemperatureSensor.h"
#include "Sensors/TemperatureObserver.h"

#include "Sensors/HumiditySensor.h"
#include "Sensors/HumidityObserver.h"

#include "Sensors/TVOCSensor.h"
#include "Sensors/TVOCObserver.h"

#include "Sensors/CO2Sensor.h"
#include "Sensors/CO2Observer.h"

#include "Sensors/ParticleSensor.h"
#include "Sensors/ParticleObserver.h"

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
      , public ParticleObserver
    {
        virtual const char* name() = 0;
    };

    static const TimeSpan DefaultInterval; // = TimeSpan(1, TimeSpan::Units::Seconds);

    explicit SmartSensor(
        const TimeSpan& pollingInterval = DefaultInterval
    ) , _pollingInterval(pollingInterval)
      , _timer(pollingInterval)
    {}
    

    void attachTemperatureSensor(const TemperatureSensor& sensor) {

    }

    void attachHumiditySensor(HumiditySensor& sensor) {

    }

    void attachTVOCSensor(const TVOCSnesor& sensor) {

    }

    void attachCO2Sensor(const CO2Sensor& sensor) {

    }

    void attachParticleSensor(const ParticleSensor& sensor) {

    }
    
    void registerObserver(Observer observer) {
        _observers.push+back(observer);
    }


    void setup()
    {
        if (_init()) {
            Log.infoln("BME680 initialised.");
            _sensor.beginReading();
            _timer.restart();
            _present = true;
        }

        else {
            Log.errorln("BME680 failed to initialise - not connected?");
        }
    }

    void loop()
    {
        if (_present)
        {
            if (_timer.hasExpired() && _sensor.remainingReadingMillis() == 0) 
            {
                _sensor.endReading();

                _processTemperature(_sensor.readTemperature());
                _processHumidity(_sensor.readHumidity());
                _sensor.readGas();

                // Read and tell observers.
                // then...

                _sensor.beginReading();
                _timer.restart();
            }
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

    typedef std::list<Observer> _Observers;

    Adafruit_BME680 _Sensor;
    int _i2cAddress;

    TimeSpan _pollingInterval;
    bool _present; 

    CountdownTimer _timer;

    _Observers _observer;
};

#endif
