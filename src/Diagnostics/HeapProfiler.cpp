#include "Diagnostics/HeapProfiler.h"
#include "Diagnostics/Logging.h"

HeapProfiler::HeapProfiler(uint32_t samplePeriod) 
    : _samplePeriod(samplePeriod)
    , _timer(samplePeriod, _Timer::State::Running)
    , _observer()
{}

void HeapProfiler::setObserver(Observer observer) {
    _observer = observer;
}

void HeapProfiler::loop()
{
    if (_timer.hasExpired()) {
        if (_observer) {
            // Note: ESP.getFreeHeap() shows the amount of free heap since last call to this function.
            // while ESP.getMinFreeHeap() shows the lowest point free heap since last call to this function.
            // Its more useful to show the lowest the free heap got since the last call as this
            // shows how close to the wind we've sailed.
            _observer(ESP.getHeapSize(), ESP.getMinFreeHeap());
        }
        _timer.restart();
    }
}