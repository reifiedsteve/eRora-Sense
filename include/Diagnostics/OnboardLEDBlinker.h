#pragma once

#include <Arduino.h>

#include "Chronos/CountdownTimer.h"

#include "Diagnostics/Logging.h"

class OnboardLEDBlinker
{
public:

    typedef CountdownTimer::Ticks Ticks;

    static const uint8_t DefaultPinNo = 2;

    inline explicit OnboardLEDBlinker(uint8_t pinNo = DefaultPinNo) __attribute__((always_inline))
      : _ledPinNo(pinNo)
      , _state(_State::Idle)
      , _onTimeMs(500)
      , _offTimeMs(250)
      , _onTimer(_onTimeMs)
      , _offTimer(_offTimeMs)
      , _blinksToGo(0)
    {
        pinMode(_ledPinNo, OUTPUT);
    }

    /// @brief Specify the (default) time the LED will be on during a blink.
    /// @param ms The time in milliseconds.
    inline void setOnTime(Ticks ms) __attribute__((always_inline)) {
        _onTimeMs = ms;
        _onTimer.setCountdown(ms);
    }

    /// @brief Specify the (default) time the LED will be off during a blink.
    /// @param ms The time in milliseconds.
    inline void setOffTime(Ticks ms) __attribute__((always_inline)) {
        _offTimeMs = ms;
        _offTimer.setCountdown(ms);
    }

    /// @brief Start a number of blinks, using the defined default on/off times.
    /// @param numBlinks The number of blinks.
    void start(unsigned numBlinks);

    /// @brief Start a number of blinks, using the now given on/off blink times
    /// (note: does *not* change the default times).
    /// @param numBlinks The number of blinks.
    /// @param onOffTimeMs The on/off time in milliseconds.
    void start(unsigned numBlinks, int onOffTimeMs);

    /// @brief Start a number of blinks, using the now given on/off blink times
    /// (note: does *not* change the default times).
    /// @param numBlinks The number of blinks.
    /// @param onTimeMs The on time in mulliseconds.
    /// @param offTimeMs The off time in millisevconds.
    void start(unsigned numBlinks, int onTimeMs, int offTimeMs);

    inline bool isRunning() const __attribute__((always_inline)) {
        return _state != _State::Idle;
    }

    void loop();

protected:

    enum class _State : uint8_t {
        Idle, On, Off
    };

private:

    void _start(unsigned numBlinks, int onTimeMs, int offTimeMs);

    void _goToIdleState() {
        _off();
        _onTimer.stop();
        _offTimer.stop();
        _onTimer.reset();
        _offTimer.reset();
        _blinksToGo = 0;
    }

    void _goToOnState() {
        _on();
        _onTimer.restart();
        _state = _State::On;
    }

    void _goToOffState() {
        _off();
        _offTimer.restart();
        _state = _State::Off;
    }

    void _on() {
        digitalWrite(_ledPinNo, HIGH);
        // Log.verboseln("OnboardLEDBlinker: now on.");
    }

    void _off() {
        digitalWrite(_ledPinNo, LOW);
        // Log.verboseln("OnboardLEDBlinker: now off.");
    }

    uint8_t _ledPinNo;   
    _State _state;

    int _onTimeMs;
    int _offTimeMs;

    CountdownTimer _onTimer;
    CountdownTimer _offTimer;

    int _blinksToGo;
};

