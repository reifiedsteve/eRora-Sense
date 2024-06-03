#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <esp_system.h>
#include <driver/ledc.h>

class FanController
{
public:

    explicit FanController(uint8_t pwmPin,  uint32_t frequency = 25000, uint8_t resolution = 8, uint8_t channelNo = 0)
      : _pwmPin(pwmPin)
      , _frequency(frequency)
      , _resolution(resolution)
      , _channelNo(channelNo)
    {}

    void begin() {
        ledcAttachPin(_pwmPin, _channelNo);
        ledcSetup(_channelNo, _frequency, _resolution); // Use 25Khz at 8-bit resolution.
        ledcWrite(_channelNo, 0); // full speed (for 8-bit resolution).
    }

    void setDutyCycle(float percentage) {
        ledcWrite(_channelNo,  (uint32_t)((1 << _resolution)-1) * (percentage / 100.0));
    }

private:

    uint8_t _pwmPin;
    uint32_t _frequency;
    uint8_t _resolution;
    uint8_t _channelNo;
};
