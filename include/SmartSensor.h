#pragma once

#include <list>

#include "SmartSensorObserver.h"

#include "Sensors/BME680Sensor.h"

#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

#include "Fan/FanController.h"

class SmartSensor
{
public:

    typedef SmartSensorObserver Observer;

    static const TimeSpan DefaultInterval;

    SmartSensor();
    
    void bindObserver(Observer& observer);
    void bindFanController(FanController& controller);
    // void bindFanSpeedObserver();

    void switchPower(bool on);
    void togglePower();

    void selectNextMode();

    /// @brief Set the fan speed (for when fan is in manual mode).
    /// @param speedSetting Setting of 0-10 (off to maximum).
    void setFanSpeed(uint8_t speedSetting);

    void adjustFanSpeed(int8_t delta);

    /// @brief Increase fan speed (for when fan is in manual mode).
    void increaseFanSpeed();

    /// @brief Decrease fan speed (for when fan is in manual mode).
    void decreaseFanSpeed();

    void setup();
    void loop();

private:

    bool _setFanSpeed(uint8_t speedSetting);

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

    void _informOfFanSpeed(uint8_t fanSpeed);

    typedef std::list<Observer*> _Observers;

    BME680Sensor _bmeSensor;
    // SGP30Sensor _sgpSensor;

    float _temperature, _relHumidity;
    float _co2, _tvoc;
    
    bool _iaqAvailable;
    float _iaq;

    CountdownTimer _timer;
    _Observers _observers;

    FanController* _fanController;
    int _fanSpeed;
};

