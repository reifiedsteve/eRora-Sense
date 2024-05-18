#pragma once

#include <list>

#include "SmartSensorObserver.h"

#include "Sensors/BME680Sensor.h"

#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

class SmartSensor
{
public:

    typedef SmartSensorObserver Observer;

    static const TimeSpan DefaultInterval;

    SmartSensor();
    
    void bindObserver(Observer& observer);

    void setup();
    void loop();

private:

    void _processTemperature(float temperature);
    void _processHumidity(float relHumidity);
    void _processTVOC(float tvoc);
    void _processCO2(float co2);
    void _processIAQ(bool available, float iaq);

    void _informOfTemperature(float temperature);
    void _informOfHumidity(float humidity);
    void _informOfTVOC(float tvoc);
    void _informOfCO2(float co2);
    void _informOfIAQ(float iaq);

    void _informOfIAQAvailability(bool available);

    typedef std::list<Observer*> _Observers;

    BME680Sensor _bmeSensor;
    // SGP30Sensor _sgpSensor;

    float _temperature, _relHumidity;
    float _co2, _tvoc, _iaq;

    bool _iaqAvailable;

    CountdownTimer _timer;

    _Observers _observers;
};

