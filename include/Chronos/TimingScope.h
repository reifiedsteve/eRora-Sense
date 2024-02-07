#pragma once

#include "Chronos/Stopwatch.h"

class TimingScope
{
public:

    inline explicit TimingScope(Stopwatch& timer) : _timer(timer) { 
        _timer.start();
    }

    inline ~TimingScope() {
        _timer.stop();
    }

private:

    Stopwatch& _timer;
};
