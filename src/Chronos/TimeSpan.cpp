#include "Chronos/TimeSpan.h"

const TimeSpan TimeSpan::Zero = TimeSpan(0, Units::Ticks);

void TimeSpan::_set(Ticks span, Units units)
{
    switch (units) {
        case Units::Ticks:
        case Units::Microseconds:
            _ticks = span;
            break;
        case Units::Milliseconds:
            _ticks = _ticksPerMillisecond * span;
            break;
        case Units::Seconds:
            _ticks = _ticksPerSecond * span;
            break;
        case Units::Minutes:
            _ticks = _ticksPerSecond * _secondsPerMinute * span;
            break;
        default:
            Log.errorln("TimeSpan: set - unknown units.");
            _ticks = 0;
            break;
    }
}

void TimeSpan::_setPrecise(double span, Units units)
{
    switch (units) {
        case Units::Ticks:
        case Units::Microseconds:
            _ticks = (Ticks)span;
            break;
        case Units::Milliseconds:
            _ticks = (Ticks)(_ticksPerMillisecond * span);
            break;
        case Units::Seconds:
            _ticks = (Ticks)(_ticksPerSecond * span);
            break;
        case Units::Minutes:
            _ticks = (Ticks)(_ticksPerSecond * _secondsPerMinute * span);
            break;
        default:
            Log.errorln("TimeSpan: set - unknown units.");
            _ticks = 0;
            break;
    }
}

TimeSpan::Ticks TimeSpan::_get(Units units) const 
{
    Ticks span(0);

    switch (units) {
        case Units::Ticks:
            span = _ticks;
            break;        
        case Units::Microseconds:
            span = _ticks;
            break;        
        case Units::Milliseconds:
            span = _ticks / _ticksPerMillisecond;
            break;
        case Units::Seconds:
            span = _ticks / _ticksPerSecond;
            break;
        case Units::Minutes:
            span = _ticks / _ticksPerSecond / _secondsPerMinute;
            break;
        default:
            Log.errorln("TimeSpan: get - unknown units.");
            break;
    }

    return span;
}

double TimeSpan::_getPrecise(Units units) const 
{
    double span(0.0);

    switch (units) {
        case Units::Ticks:
            span = (double) _ticks;
            break;        
        case Units::Microseconds:
            span = (double) _ticks;
            break;        
        case Units::Milliseconds:
            span = (double) _ticks / _ticksPerMillisecond;
            break;
        case Units::Seconds:
            span = (double) _ticks / _ticksPerSecond;
            break;
        case Units::Minutes:
            span = (double) _ticks / _ticksPerSecond / _secondsPerMinute;
            break;
        default:
            Log.errorln("TimeSpan: get - unknown units.");
            break;
    }

    return span;
}

