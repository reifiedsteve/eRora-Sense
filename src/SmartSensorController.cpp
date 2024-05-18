#include "SmartSensorController.h"

SmartSensorController::SmartSensorController(SmartSensor& sensor, const char* controllerName, const TimeSpan& executionInterval) 
  : _sensor(sensor)
  , _controllerTimer(executionInterval.millis())
  , _operations()
{}    


void SmartSensorController::setup() {
    _initInputs();
}

void SmartSensorController::loop() {
    _serviceInputs();
    _executeDeferredOperations(_operations.size());
}

void SmartSensorController::_switchOnOff(bool on) {
    _scheduleOperation([this, on]() {
        //_sensor.switchPower(on);
    });
}

void SmartSensorController::_toggleOnOff() {
    _scheduleOperation([this]() {
        //_sensor.togglePower();
    });
}

void SmartSensorController::_selectNextDisplayMode() {
    _scheduleOperation([this]() {
        //_sensor.selectNextMode();
    });
}

void SmartSensorController::_setFanSpeed(uint8_t speed) {
    _scheduleOperation([this, speed]() {
        //_sensor.setFanSpeed(speed]);
    });
}

void SmartSensorController::_scheduleOperation(_Operation command) {
    _operations.push(command);
}

size_t SmartSensorController::_executeDeferredOperations(size_t n)
{
    bool executing(true);
    int i(0);

    do {
        executing = executing && _executeDeferredOperation();
    } while (executing && (i++ <= n));

    return i;
}

bool SmartSensorController::_executeDeferredOperation() {
    bool executed(false);
    if (_operations.size() > 0) {
        _Operation command(_operations.front());
        _operations.pop();
        command();
        executed = true;
    }
    return executed;
}

bool SmartSensorController::_allDigits(const std::string& str) {
    return str.find_first_not_of("0123456789") == std::string::npos;
}

bool SmartSensorController::_meansTrue(const std::string& str) {
    return (str == "true") || (str == "on") || (str == "enable") || (str == "enabled") || (str == "1");
}

bool SmartSensorController::_meansFalse(const std::string& str) {
    return (str == "false") || (str == "off") || (str == "disable")|| (str == "disabled") || (str == "0");
}

