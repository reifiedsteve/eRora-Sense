
#include "Diagnostics/FrameRateProfiler.h"
#include "Diagnostics/Logging.h"

FrameRateProfiler::FrameRateProfiler(const char* name, uint32_t samplePeriod, bool report) 
    : _name(name)
    , _samplePeriod(samplePeriod)
    , _report(report)
    , _stopwatch()
    , _frames(0)
    , _observer()
    , _fps(0)
{}

void FrameRateProfiler::setObserver(Observer observer) {
    _observer = observer;
}

void FrameRateProfiler::enable(bool report) {
    _report = report;
}

void FrameRateProfiler::advanceOneFrame()
{
    if (!_stopwatch.isRunning()) {
        _frames = 0;
        _stopwatch.restart();
    }

    ++_frames;

    if (_stopwatch.elapsed() > _samplePeriod)
    {
        _stopwatch.stop();

        float secs(_samplePeriod / 1000.0);
        _fps = (_Ticks)((_samplePeriod == 0) ? 0 : (_frames / secs));
        
        if (_report) {
            #ifdef DISABLE_LOGGING
            std::cout << "Performance [" << _name << "]: " << _fps << " FPS" << std::endl << std::flush;
            #else
            Log.infoln("Performance [%s]: %d FPS (Free heap: %d, min %d).", _name, _fps, esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
            #endif
            if (_observer) {
                _observer(_fps);
            }
        }
    }
}

int32_t FrameRateProfiler::framesPerSecond() const {
    return _fps;
}
