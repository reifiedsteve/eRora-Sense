#pragma once

#include "SmartSensor.h"
#include "SmartSensorController.h"
#include "Drivers/MomentaryButton.h"
#include "Chronos/CountdownTimer.h"

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
    );

private:

    void _initInputs() override;

    void _serviceInputs() override;

    Button& _powerButton;
    Button& _modeButton;
    Button& _fanDecButton;
    Button& _fanIncButton;

    CountdownTimer _repeatTimer;
    CountdownTimer _modeCycleTimer;
};
