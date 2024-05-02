#pragma once

#include <inttypes.h>
#include <queue>

#include "SmartSensor.h"

class SmartSensorController
{
public:

    explicit SmartSensorController(SmartSensor& sensor);

    void loop();

protected:

    void _switchPower(bool on);
    void _togglePower();

    void _selectNextDisplayMode();

    /// Set the fan to speed 0-10, 0 being fully off, 10 being full speed.
    void _setFanSpeed(uint8_t speed);
    bool _adjustFanSpeed(int delta);

private:

    typedef std::function<void()> _Command;
    typedef std::queue<_Command> _Commands;

    void _deferCommand(_Command command);
    void _executeCommands(size_t n);
    void _executeNextCommand();

    SmartSensor& _sensor;
    _Commands _commands;
};