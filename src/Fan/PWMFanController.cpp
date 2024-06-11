#include <cmath>

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <esp_system.h>
#include <driver/ledc.h>

#include "Diagnostics/Logging.h"

#include "Fan/PWMFanController.h"

PWMFanController::PWMFanController(uint8_t pwmPin, uint32_t pwmFrequency, uint8_t pwmResolution, uint8_t pwmChannelNo )
    : _isOn(false)
    , _dutyCyclePercentage(100)
    , _pwmPin(pwmPin)
    , _pwmFrequency(pwmFrequency)
    , _pwmResolution(pwmResolution)
    , _pwmChannelNo(pwmChannelNo)
    , _useSeparateOnOffControlPin(false)
    , _onOffPin(0)
    , _minSpeedLimit(0) 
    , _maxSpeedLimit(100)
{}

/// @brief For fans that are continuous motion (i.e. 0% duty cycle still
/// results in motion), we can use a separate GPIO yto (e.g.) control a
/// MOSFET to physically turn the fan off by cutting power to it.
/// @param onOffPin The pin to control the MOSFET/transistor.
void PWMFanController::configSeparatePowerControlPin(uint8_t onOffPin) {
    Log.verboseln("PWMFanController: setting power control pin (MOSFET) to pin %d.", onOffPin);
    _onOffPin = onOffPin;
    pinMode(_onOffPin, OUTPUT);
    _useSeparateOnOffControlPin = true;
}

/// @brief Set the limits of speed as a percentage of the actual physical maximum speed.
/// @param minMotionPercent The percentage of max possible physical speed to be
/// logically considered the minumum requestable percent (i.e. for set speed of 0, where
/// the fan is in minimal motion but still moving). Valid range is 0 - 100.
/// @param maxMotionPercent The percentage of max possible physical speed to be
/// logically considered the maximum requestable percent (i.e. for set speed of 100).
/// Can be used to cap the maximum possible requested speed (e.g. if fan is noisy at
/// its maximum and this is less desirable that capping its speed). Valid range is 0 - 100.
void PWMFanController::limitPhysicalSpeedRange(uint8_t minMotionPercent, uint8_t maxMotionPercent) {
    _minSpeedLimit = std::min(minMotionPercent, maxMotionPercent);
    _maxSpeedLimit = std::max(minMotionPercent, maxMotionPercent);
}

/// @brief Initialise the fan.
void PWMFanController::begin() {
    _init();
}

/// @brief Turn the fan on or off, where off means zero motion.
/// @param on Turn on if true; otherwise turn off.
void PWMFanController::setPower(bool on) {
    _setPower(on);
    _isOn = on;
}

/// @brief Set the fan speed expressed as duty cycle, a percentage. 0 -100. 
/// 0 means minimum (in motion) speed. 100 means maximum allowed speed.
/// @param percentage 0 (minimum) - 100 (maximum).
void PWMFanController::setSpeed(uint8_t percentage)
{
    _dutyCyclePercentage = std::min(percentage, (uint8_t)100);

    if (_isOn) {
        _setDutyCyclePercent(_dutyCyclePercentage);
    } else {
        _setDutyCycleValue(0);
    }
}

void PWMFanController::_init() {
    pinMode(_pwmPin, OUTPUT);
    ledcSetup(_pwmChannelNo, _pwmFrequency, _pwmResolution); // Use 25Khz at 8-bit resolution.
    ledcAttachPin(_pwmPin, _pwmChannelNo);
    Log.infoln("PWMFanController: configured PWM channel %d, freq %d, resolution %d.", _pwmChannelNo, _pwmFrequency, _pwmResolution);
    ledcWrite(_pwmChannelNo, 0); // fan is initially at minimum speed.
    Log.infoln("PWNConbtroller: initial duty cycle set to 0.");
}

void PWMFanController::_setPower(bool on)
{
    if (_useSeparateOnOffControlPin) {
        // For fans that are not motionless at 0 duty cycle and require a (e.g.) relay/MOSFET to be turned off.
        Log.verboseln("PWMFanController: switching fan on/off switch %s.", on ? "on" : "off");
        digitalWrite(_onOffPin, on ? true : false);
    }
    
    // For fans that are motionless at 0% duty cycle and
    // do not require a separate GPIO controller on/off switch.
    // In which case going to 0% duty cycle is the best we can do.

    if (on) {
        _setDutyCyclePercent(_dutyCyclePercentage);
    } else {
        _setDutyCycleValue(0);
    }
}

void PWMFanController::_setDutyCyclePercent(uint8_t speedPercentage)
{
    uint32_t dutyCycleMaxPossible(_maxValueForResolution(_pwmResolution));

    uint32_t dutyCycleMin(0);
    uint32_t dutyCycleMax(dutyCycleMaxPossible);

    // Remap duty cycle limits of the values as specified by percentages
    // of full duty cycle range.

    if (_minSpeedLimit != 0) {
        dutyCycleMin = (uint32_t)(dutyCycleMaxPossible * (_minSpeedLimit / 100.0));
    }

    if (_maxSpeedLimit != 100) {
        dutyCycleMax = (uint32_t)(dutyCycleMaxPossible * (_maxSpeedLimit / 100.0));
    }

    uint32_t dutyCycleVal((uint32_t)(dutyCycleMin + (dutyCycleMax - dutyCycleMin) * (speedPercentage / 100.0)));

    Log.verboseln("PWMFanController: fan speed %d pc (duty cycle value of %d).", (int)speedPercentage, dutyCycleVal);

    _setDutyCycleValue(dutyCycleVal);
}

void PWMFanController::_setDutyCycleValue(uint32_t dutyCycleVal) {
    ledcWrite(_pwmChannelNo, dutyCycleVal);
    Log.verboseln("PWMFanController: set duty cycle of %d (max is %d).", (int)dutyCycleVal, _maxValueForResolution(_pwmResolution));
}

uint32_t PWMFanController::_maxValueForResolution(uint32_t resolution) {
    return (1 << resolution) - 1;
}
