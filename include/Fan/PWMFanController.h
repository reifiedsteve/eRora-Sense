#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <esp_system.h>
#include <driver/ledc.h>

#include "Diagnostics/Logging.h"

#include "Fan/FanController.h"

class PWMFanController : public FanController
{
public:

    explicit PWMFanController(uint8_t pwmPin,  uint32_t pwmFrequency = 25000, uint8_t pwmResolution = 8, uint8_t pwmChannelNo = 0)
      : _pwmPin(pwmPin)
      , _pwmFrequency(pwmFrequency)
      , _pwmResolution(pwmResolution)
      , _pwmChannelNo(pwmChannelNo)
    {}

    void setup() {
        pinMode(_pwmPin, OUTPUT);
        ledcSetup(_pwmChannelNo, _pwmFrequency, _pwmResolution); // Use 25Khz at 8-bit resolution.
        ledcWrite(_pwmChannelNo, 0); // fan is initially off.
    }

    void setFanSpeed(uint8_t speedPercentage) override {
        uint32_t dutyCycleVal((uint32_t)(((1 << _pwmResolution)-1) * (speedPercentage / 100.0)));
        Log.verboseln("FanController: fan speed %d pc (duty cycle of %d).", (int)speedPercentage, dutyCycleVal);
        ledcWrite(_pwmChannelNo,  dutyCycleVal);
    }

private:

    uint8_t _pwmPin;
    uint32_t _pwmFrequency;
    uint8_t _pwmResolution;
    uint8_t _pwmChannelNo;
};
