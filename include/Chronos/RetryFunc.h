#pragma once

#include <functional>
#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

class RetryFunc
{
public:

    typedef std::function<bool()> Function;

    RetryFunc(Function func, const TimeSpan& interval)
      : _func(func)
      , _complete(false)
      , _timer(interval.millis(), CountdownTimer::State::Running)
    {}

    bool tryFunction() {
        if (!_complete) {
            if (_timer.hasExpired()) {
                _complete = _func();
                _timer.restart();
            }
        }
        return _complete;
    }

    operator bool() {
        return tryFunction();
    }

private:

    Function _func;
    bool _complete;
    CountdownTimer _timer;
};
