#pragma once

#include <inttypes.h>

#include "Fan/FanController.h"

class PWMFanController : public FanController
{
public:

    explicit PWMFanController(uint8_t pwmPin, uint32_t pwmFrequency = 25000, uint8_t pwmResolution = 8, uint8_t pwmChannelNo = 0);

    /// @brief For fans that are continuous motion (i.e. 0% duty cycle still
    /// results in motion), we can use a separate GPIO yto (e.g.) control a
    /// MOSFET to physically turn the fan off by cutting power to it.
    /// @param onOffPin The pin to control the MOSFET/transistor.
    void configSeparatePowerControlPin(uint8_t onOffPin);

    /// @brief Set the limits of speed as a percentage of the actual physical maximum speed.
    /// @param minMotionPercent The percentage of max possible physical speed to be
    /// logically considered the minumum requestable percent (i.e. for set speed of 0, where
    /// the fan is in minimal motion but still moving). Valid range is 0 - 100.
    /// @param maxMotionPercent The percentage of max possible physical speed to be
    /// logically considered the maximum requestable percent (i.e. for set speed of 100).
    /// Can be used to cap the maximum possible requested speed (e.g. if fan is noisy at
    /// its maximum and this is less desirable that capping its speed). Valid range is 0 - 100.
    void limitPhysicalSpeedRange(uint8_t minMotionPercent, uint8_t maxMotionPercent);

    /// @brief Initialise the fan.
    void begin();

    /// @brief Turn the fan on or off, where off means zero motion.
    /// @param on Turn on if true; otherwise turn off.
    void setPower(bool on) override;

    /// @brief Set the fan speed as a perventage. 0 -100. 
    /// 0 means minimum (in motion) speed. 100 means maximum allowed speed.
    /// @param speedPercentage 0 (minimum) - 100 (maximum).
    void setFanSpeed(uint8_t speedPercentage) override;

private:

    void _init();

    void _setPower(bool on);
    void _setSpeed(uint8_t speedPercentage);
    void _setDutyCycle(uint32_t dutyCycleVal);

    static uint32_t _maxValueForResolution(uint32_t resolution);

    bool _isOn;
    uint8_t _speedPercent;

    uint8_t _pwmPin;
    uint32_t _pwmFrequency;
    uint8_t _pwmResolution;
    uint8_t _pwmChannelNo;

    bool _useSeparateOnOffControlPin;
    uint8_t _onOffPin;

    uint32_t _minSpeedLimit;
    uint32_t _maxSpeedLimit;
};
