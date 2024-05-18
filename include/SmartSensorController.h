#pragma once

#include <inttypes.h>
#include <queue>

#include "SmartSensor.h"

class SmartSensorController
{
public:

    explicit SmartSensorController(SmartSensor& sensor);

    void setup();
    void loop();

protected:

    SmartSensorController(SmartSensor& sensor, const char* controllerName, const TimeSpan& executionInterval);

    virtual void _initInputs() = 0;
    virtual void _serviceInputs() = 0;

    // Powering on or off refers to the fan/cabinet-backlight
    // (and maybe the LCD backlight).
    // The sensors continue to sample and publish to all
    // observers regardless.

    void _switchOnOff(bool on); // on/off of backlight and fan.
    void _toggleOnOff();

    /// Set the fan to speed 1-10, 1 being minimum, 10 being full speed.
    void _setFanSpeed(uint8_t speed);
    bool _adjustFanSpeed(int delta);

    // Set the cabinet backlight brightness, 0 being minimum,
    // 255 being full brightness.
    void _setBacklightBrightness(uint8_t brightness);
    void _setBacklightColour(uint8_t hue, uint8_t sat);

    // void _setDisplayMode(DisplayMode mode); // TODO:
    void _selectNextDisplayMode();
    
    static bool _allDigits(const std::string& str);

    static bool _meansTrue(const std::string& str);
    static bool _meansFalse(const std::string& str);

private:

    typedef std::function<void()> _Operation;
    typedef std::queue<_Operation> _Operations;

    void _scheduleOperation(_Operation command);
    size_t _executeDeferredOperations(size_t n);
    bool _executeDeferredOperation();
    
    SmartSensor& _sensor;

    CountdownTimer _controllerTimer;
    _Operations _operations;
};