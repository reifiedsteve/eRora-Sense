#pragma once

#include <functional>

#include "Chronos/Stopwatch.h"

class FrameRateProfiler
{
public:

    typedef std::function<void(unsigned fps)> Observer;

    explicit FrameRateProfiler(const char* name, uint32_t samplePeriod = 1000, bool report = true);

    void setObserver(Observer observer);
    void enable(bool report);

    void advanceOneFrame();

    int32_t framesPerSecond() const;
    
private:

    typedef Stopwatch _Stopwatch;
    typedef _Stopwatch::Ticks _Ticks;

    const char* _name;
    _Ticks _samplePeriod;
    bool _report;

    _Stopwatch _stopwatch;
    uint32_t _frames;

    Observer _observer;

    uint32_t _fps;
};
