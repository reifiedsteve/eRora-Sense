#pragma once

#include "SmartSensor.h"
#include "Controls/MomentaryButton.h"
#include "SmartSensorController.h"
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
        Button& fanControl1Button,
        Button& fanControl2Button
    )
      : SmartSensorController(smartSensor, "button panel", loopInterval)
      , _powerButton(powerButton)
      , _modeButton(modeButton)
      , _fanIncButton(fanControl1Button)
      , _fanDecButton(fanControl2Button)
    {}

private:

    void _initInputs() override 
    {
        Log.verboseln("ButtonController: binding semantics to buttons...");

        _fanIncButton.attachClick([this](){
            Log.infoln("ButtonController: ##########------> fan increase button pressed.");
            _adjustFanSpeed(+1);
        });

        _fanDecButton.attachClick([this](){
            Log.infoln("ButtonController: ##########------> fan decrease button pressed.");
            _adjustFanSpeed(-1);
        });

        _fanIncButton.attachDoubleClick([this]() { // TODO: long press instead?
            Log.infoln("ButtonController: ##########------> fan toggle mode [1] button pressed.");
            // TODO: toggle fan mode.
        });
        
        _fanDecButton.attachDoubleClick([this]() { // TODO: long press instead?
            Log.infoln("ButtonController: ##########------> fan toggle mode [2] button pressed.");
            // TODO: toggle fan mode.
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
    Button& _fanIncButton;
    Button& _fanDecButton;
};
