
#include "Controllers/ButtonController.h"
#include "Diagnostics/Logging.h"

    ButtonController::ButtonController(
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
      , _repeatTimer(500)
      , _modeCycleTimer(1000)
    {
        static const int singleClickMs = 400; // TODO: try smaller values for quicker responses.
        _powerButton.setClickMs(singleClickMs);
        _modeButton.setClickMs(singleClickMs);
        _fanDecButton.setClickMs(singleClickMs);
        _fanIncButton.setClickMs(singleClickMs);
    }

    void ButtonController::_initInputs()
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
            _modeCycleTimer.restart();
        });
        
        _modeButton.attachDuringLongPress([this]() { 
            if (_modeCycleTimer.hasExpired()) {
                _selectNextDisplayMode();
                _modeCycleTimer.restart();
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
            _repeatTimer.restart();
        });
        
        _fanIncButton.attachDuringLongPress([this]() { 
            if (_repeatTimer.hasExpired()) {
                _adjustFanSpeed(+1);
                _repeatTimer.restart();
            }
        });
        
        _fanIncButton.attachLongPressStop([this]() { 

        });

        _fanDecButton.attachLongPressStart([this]() {
            _adjustFanSpeed(-1);
            _repeatTimer.restart();
        });

        _fanDecButton.attachDuringLongPress([this]() {
            if (_repeatTimer.hasExpired()) {
                _adjustFanSpeed(-1);
                _repeatTimer.restart();
            }
        });

        _fanDecButton.attachLongPressStop([this]() { 

        });
    }

    void ButtonController::_serviceInputs() {
        _powerButton.tick();
        _modeButton.tick();
        _fanIncButton.tick();
        _fanDecButton.tick();
    }
