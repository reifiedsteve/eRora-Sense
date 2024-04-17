#include "Sensors/PMS7003ParticleSensor.h"

const TimeSpan PMS7003ParticleSensor::DefaultInterval(10, TimeSpan::Units::Minutes);

PMS7003ParticleSensor::PMS7003ParticleSensor(uint8_t rxPin, uint8_t txPin, const TimeSpan& interval)
    : _pms(PMS7003, rxPin, txPin)
    , _state(_State::Init)
    , _available(false)
    , _measurements()
    , _timer(interval.millis(), CountdownTimer::State::Running)
{}

void PMS7003ParticleSensor::setup() {
    _pms.init();
    _pms.sleep();
}

bool PMS7003ParticleSensor::available() {
    _available = _available || _cycle();
    return _available;
}

const PMS7003ParticleSensor::Measurements& PMS7003ParticleSensor::read() {
    _available = false;
    return _measurements;
}

bool PMS7003ParticleSensor::_cycle()
{
    bool sampleAvailable(false);

    switch (_state)
    {
        case _State::Init:
            _cycleInit();
            break;

        case _State::Sleeping:
            _cycleSleeping();
            break;

        case _State::Reading:
            sampleAvailable = _cycleReading();
            break;
    }

    return sampleAvailable;
}

void PMS7003ParticleSensor::_cycleInit() {
    _pms.wake();
    _state = _State::Reading;
}

void PMS7003ParticleSensor::_cycleSleeping() {
    if (_timer.hasExpired()) {
        _state = _State::Reading;
    }
}

bool PMS7003ParticleSensor::_cycleReading() {
    bool available(false);
    if (_pms.has_particulate_matter()) {
        _measurements.pm01 = _pms.pm01;
        _measurements.pm25 = _pms.pm25;
        _measurements.pm10 = _pms.pm10;
        _pms.sleep();
        _timer.restart();
        _state = _State::Sleeping;
        available = true;
    }
    return available;
}
