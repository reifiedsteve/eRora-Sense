#include "SmartSensorController.h"

SmartSensorController::SmartSensorController(SmartSensor& sensor, const char* controllerName, const TimeSpan& executionInterval, Responsiveness responsiveness) 
  : _smartSensor(sensor)
  , _controllerName(controllerName)
  , _responsiveness(responsiveness)
  , _responsiveWhenOff(_responsiveness == Responsiveness::Always)
  , _controllerTimer(executionInterval.millis())
  , _operationsMutex()
  , _deferredOperations()
{}    

const char* SmartSensorController::controllerName() const {
    return _controllerName;
}

void SmartSensorController::setLoopInterval(Ticks ticks) {
    _controllerTimer.setCountdown(ticks);
    _controllerTimer.restart();
}

SmartSensorController::Ticks SmartSensorController::getLoopInterval() const {
    return _controllerTimer.getCountdown();
}

bool SmartSensorController::responsiveWhenOff() const {
    return _responsiveness == Responsiveness::Always;
}

void SmartSensorController::setup() {
    _initInputs();
    _controllerTimer.start(); // Safe to start this in the ctor instead?
}

void SmartSensorController::loop()
{
    if (_controllerTimer.hasExpired()) {
        _controllerTimer.restart();
        _serviceInputs();
        #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
        _executeDeferredOperations();
        #endif
    }
}

void SmartSensorController::_switchOnOff(bool on) {
    Log.verboseln("SmartSensorController: queuing switchOnOff(%s) - from controller '%s'.", (on ? "on" : "off"), _controllerName);
    _scheduleOperation([this, on]() {
        _smartSensor.switchPower(on);
    });
}

void SmartSensorController::_toggleOnOff() {
    Log.verboseln("SmartSensorController: queuing toggleOnOff - from controller '%s'.", _controllerName);
    _scheduleOperation([this]() {
        _smartSensor.togglePower();
    });
}

void SmartSensorController::_selectNextDisplayMode() {
    Log.verboseln("SmartSensorController: queuing selectNextDisplayMode() - from controller '%s'.", _controllerName);
    #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
    _scheduleOperation([this]() {
        if (_smartSensor.isOn() || _responsiveWhenOff) {
            _smartSensor.selectNextDisplayMode();
        }
    });
    #else
    if (_smartSensor.isOn() || _responsiveWhenOff) {
        _smartSensor.selectNextDisplayMode();
    }
    #endif
}

void SmartSensorController::_reboot() {
    Log.noticeln("SmartSensorController: received reboot request (from %s)...", _controllerName);
    #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
    _scheduleOperation([this]() {
        _smartSensor.reboot();
    });
    #else
    _smartLight.reboot();
    #endif
}

void SmartSensorController::_setFanSpeed(uint8_t speed) {
    Log.verboseln("SmartSensorController: queuing setFanSpeed(%d) - from controller '%s'.", speed, _controllerName);
    #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
    _scheduleOperation([this, speed]() {
        if (_smartSensor.isOn() || _responsiveWhenOff) {
            _smartSensor.setFanSpeed(speed);
        }
    });
    #else
    if (_smartSensor.isOn() || _responsiveWhenOff) {
        _smartSensor.setFanSpeed(speed);
    }
    #endif
}

 void SmartSensorController::_adjustFanSpeed(int8_t delta) {
    Log.verboseln("SmartSensorController: queuing adjustFanSpeed(%d) - from controller '%s'.", delta, _controllerName);
    #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
    _scheduleOperation([this, delta]() {
        if (_smartSensor.isOn() || _responsiveWhenOff) {
            _smartSensor.adjustFanSpeed(delta);
        }
    });
    #else
    if (_smartSensor.isOn() || _responsiveWhenOff) {
        _smartSensor.adjustFanSpeed(delta);
    }
    #endif
 }

void SmartSensorController::_triggerInspection() {
    Log.verboseln("SmartSensorController: queuing triggerInspection - from controller '%s'.", _controllerName);
    #ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD
    _scheduleOperation([this]() {
        if (_smartSensor.isOn() || _responsiveWhenOff) {
            _smartSensor.triggerInspection();
        }
    });
    #else
    if (_smartSensor.isOn() || _responsiveWhenOff) {
        _smartSensor.triggerInspection();
    }
    #endif
}

#ifdef OPERATIONS_EXECUTE_ON_MAIN_THREAD

void SmartSensorController::_scheduleOperation(_DeferredOperation op) {
    _ScopedLock lock(_operationsMutex);
    _deferredOperations.push(op);
    Log.verboseln("SmartSensorController: deferred an operation (there are now %d deferred ops).", _deferredOperations.size());
}

void SmartSensorController::_executeDeferredOperations()
{
    _ScopedLock lock(_operationsMutex);
    size_t n(_deferredOperations.size()); // Limit iterations upfront.

    if (n > 0) {
        Log.verboseln("SmartSensorController: executing %d deferred operations.", n);
    }
    
    for (int i=0; i < n; ++i) {
        _DeferredOperation op(_deferredOperations.front());
        _deferredOperations.pop();
        Log.verboseln("Popped a deferred op and about to execute it.");
        op();
    }
}

#endif

bool SmartSensorController::_allDigits(const std::string& str) {
    return str.find_first_not_of("0123456789") == std::string::npos;
}

bool SmartSensorController::_meansTrue(const std::string& str) {
    return (str == "true") || (str == "on") || (str == "enable") || (str == "enabled") || (str == "1");
}

bool SmartSensorController::_meansFalse(const std::string& str) {
    return (str == "false") || (str == "off") || (str == "disable")|| (str == "disabled") || (str == "0");
}

