#pragma once

#include "SmartSensor.h"
#include "SmartSensorController.h"
#include "Drivers/MomentaryButton.h"
#include "Chronos/CountdownTimer.h"
#include "Diagnostics/Logging.h"

class ButtonController : public SmartSensorController
{
public:

    typedef ::MomentaryButton Button;

    explicit ButtonController(
        SmartSensor& smartSensor,
        const TimeSpan& loopInterval,
        Button& powerButton,
        Button& modeButton,
        Button& fanDecButton,
        Button& fanIncButton
    )
      : SmartSensorController(smartSensor, "button panel", loopInterval, Responsiveness::WhenOn)
      , _powerButton(powerButton)
      , _modeButton(modeButton)
      , _fanDecButton(fanDecButton)
      , _fanIncButton(fanIncButton)
      , _timer(500)
      , _modeTimer(1000)
    {}

private:

    void _initInputs() override 
    {
        Log.verboseln("ButtonController: binding semantics to buttons...");

        _powerButton.attachClick([this]() {
            _toggleOnOff();
        });

        _powerButton.attachMultiClick([this]() {
            int n = _powerButton.getNumberClicks();
            if (n == 5) {
                // TODO: Start AP for configuration (indicate on LED display).
            }
        });

        _modeButton.attachClick([this]() {
            _triggerInspection();
        });

        _modeButton.attachLongPressStart([this]() {
            _modeTimer.restart();
        });
        
        _modeButton.attachDuringLongPress([this]() { 
            if (_modeTimer.hasExpired()) {
                _selectNextDisplayMode();
                _modeTimer.restart();
            }
        });

        _modeButton.attachLongPressStop([this]() { 

        });

        _fanDecButton.attachClick([this](){
            _adjustFanSpeed(-1);
        });

        _fanIncButton.attachClick([this](){
            _adjustFanSpeed(+1);
        });

        _fanIncButton.attachLongPressStart([this]() { 
            _adjustFanSpeed(+1);
            _timer.restart();
        });
        
        _fanIncButton.attachDuringLongPress([this]() { 
            if (_timer.hasExpired()) {
                _adjustFanSpeed(+1);
                _timer.restart();
            }
        });
        
        _fanIncButton.attachLongPressStop([this]() { 

        });

        _fanDecButton.attachLongPressStart([this]() {
            _adjustFanSpeed(-1);
            _timer.restart();
        });

        _fanDecButton.attachDuringLongPress([this]() {
            if (_timer.hasExpired()) {
                _adjustFanSpeed(-1);
                _timer.restart();
            }
        });

        _fanDecButton.attachLongPressStop([this]() { 

        });

        // TODO: etc.
    }

    void _serviceInputs() override {
        _powerButton.tick();
        _modeButton.tick();
        _fanIncButton.tick();
        _fanDecButton.tick();
    }

    Button& _powerButton;
    Button& _modeButton;
    Button& _fanDecButton;
    Button& _fanIncButton;

    CountdownTimer _timer;
    CountdownTimer _modeTimer;
};
