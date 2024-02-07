#pragma once

#include "Diagnostics/Logging.h"

#include <Adafruit_BME680.h>
#include <list>


#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define BME_DEFAULT_I2C_ADDRESS 77

#define SEALEVELPRESSURE_HPA (1013.25)

class SmartSensor
{
public:

    class Observer
    {
        virtual const char* name() = 0;

        virtual void onTemperature(float tempC) = 0;
        virtual void onHumidity(float humidityPercent) = 0;

        virtual void onNoSensor() = 0;

        /* etc */
    };

    explicit SmartSensor(TwoWire& i2cBus = Wire, int i2cAddress = BME68X_DEFAULT_ADDRESS, const TimeSpan& pollingInterval = 1000) {
      , _sensor(i2cBus, bme68x_i2cAddress)
      , _i2cAddress(i2cAddress)
      , _pollingInterval(pollingInterval)
      , _present(false)
      , _timer(pollingInterval)
    {}
    
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
