#pragma once

#include <functional>

#include "Chronos/CountdownTimer.h"

class HeapProfiler
{
public:

    typedef std::function<void(uint32_t totalHeap, uint32_t freeHeap)> Observer;

    explicit HeapProfiler(uint32_t samplePeriod = 5000);
    void setObserver(Observer observer);

    void loop();

private:

    typedef CountdownTimer _Timer;
    typedef _Timer::Ticks _Ticks;

    _Ticks _samplePeriod;

    _Timer _timer;
    Observer _observer;
};
