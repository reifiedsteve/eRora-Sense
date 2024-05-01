#include "SmartSensorController.h"

SmartSensorController::SmartSensorController(SmartSensor& sensor)
    : _sensor(sensor)
    , _commands()
{}

void SmartSensorController::loop() {
    _executeCommands(commands.size());
}

void SmartSensorController::_switchOn(bool on) {
    _deferCommand([this, on]() {
        _sensor.switchOn(on);
    });
}

void SmartSensorController::_selectNextPage() {
    _deferCommand([this]() {
        _sensor.selectNextPage();
    });
}

void SmartSensorController::_setFanSpeed(uint8_t speed) {
    _deferCommand([this, speed]() {
        _sensor.setFanSpeed(speed]);
    });
}

void SmartSensorController::_deferCommand(_Command command) {
    _commands.queue(command);
}

void SmartSensorController::_executeCommands(size_t n) {
    for (int i = 0; i < n; ++i) {
        _executeNextCommand();
    }
}

void SmartSensorController::_executeNextCommand() {
    _Command command(_commands.front());
    _commands.pop();
    command();
}
