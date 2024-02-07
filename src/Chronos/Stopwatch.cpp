#include "Chronos/Stopwatch.h"

Stopwatch::Stopwatch(State initially) 
    : _startTime(0)
    , _stopTime(0)
    , _elapsedEver({ .overflows = 0, .lastTimeSeen = 0 })
    , _totalPauseTime(0)
    , _running(false)
    , _doReset(true)
{
    if (initially == State::Running) _start();
}

void Stopwatch::elapsed(uint32_t& hours, uint8_t& minutes, uint8_t& seconds, uint16_t& millis) {
    Ticks ms(_elapsed());
    hours = ms / (Ticks)3600000; // 1000 * 60 * 60, ( == ms in an hour).
    ms = ms %  (Ticks)3600000;
    minutes = ms / 60000;   // 1000 * 60 ( == ms in a min)
    ms = ms % (Ticks)60000;
    seconds = ms / 1000;
    ms = ms % (Ticks)1000;
    millis = ms;
}

void Stopwatch::_start() {
    if (!_running) {
        Ticks now(_millis());
        if (_doReset) {
            _startTime = now;
            _doReset = false;
        } else {
            _totalPauseTime += (now - _stopTime);
        }
        _running = true;
    }
}

void Stopwatch::_stop() {
    if (_running) {
        _stopTime = _millis();
        _running = false;
    }
}

void Stopwatch::_reset() {
    _totalPauseTime = 0;
    _startTime = _stopTime = _millis();
    _doReset = true;
}

Stopwatch::Ticks Stopwatch::_elapsed() const
{
    Ticks ticks(_millis());

    if (_running) {
        ticks = ticks - _startTime - _totalPauseTime;
    } else {
        ticks = _stopTime - _startTime - _totalPauseTime;
    }

    return ticks;
}

/***
 
Stopwatch::Ticks Stopwatch::_elapsedTicks() const
{
    Ticks ticks(_ticks());

    if (_running) {
        ticks = ticks - _startTime - _totalPauseTime;
    } else {
        ticks = _stopTime - _startTime - _totalPauseTime;
    }

    return ticks;
}

***/

Stopwatch::Ticks Stopwatch::_millis() const 
{
    #if ESP32 || ARDUINO_ARCH_ESP32

    return _ticks() / 1000;

    #else

    uint32_t now(millis());   // TODO: is there a FreeRTOS alternative that is 64-bit and with longevity prior to overflow?
    if (now < _elapsedEver.lastTimeSeen) { // ...time can't flow backwards.
        ++_elapsedEver.overflows;
    }
    _elapsedEver.lastTimeSeen = now;
    return (((uint64_t)_elapsedEver.overflows) << 32) | ((uint64_t)_elapsedEver.lastTimeSeen);

    #endif
}
