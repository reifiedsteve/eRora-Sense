#pragam once

#include <functional>
#include "Chronos/CountdownTimer.h"
#include "Chronos/TimeSpan.h"

class PeriodicFunc
{
public:

    typedef std::function<void()> Function;

    PeriodicFunc(Function func, const TimeSpan& interval)
      : _func(func)
      , _timer(interval.millis(), CountdownTimer::State::Running)
    {}

    void loop() {
        if (_timer.hasExpired()) {
            _func();
            _timer.restart();
        }
    }
 
private:

    Function _func;
    CountdownTimer _timer;
};

