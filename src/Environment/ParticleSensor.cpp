#include "Environment/ParticleSensor.h"

const TimeSpan DefaultInterval(10, TimeSpan::Units::Minutes);

ParticleSensor::ParticleSensor(uint8_t rxPin, uint8_t txPin, const TimeSpan& interval)
    : _pms(PMS7003, rxPin, txPin)
    , _state(_State::Init)
    , _available(false)
    , _measurements()
    , _timer(interval.millis(), CountdownTimer::State::Running)
{}

void ParticleSensor::setup() {
    _pms.init();
    _pms.sleep();
}

bool ParticleSensor::available() {
    _available = _available || _cycle();
    return _available;
}

const ParticleSensor::Measurements& ParticleSensor::read() {
    _available = false;
    return _measurements;
}

bool ParticleSensor::_cycle()
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

void ParticleSensor::_cycleInit() {
    _pms.wake();
    _state = _State::Reading;
}

void ParticleSensor::_cycleSleeping() {
    if (_timer.hasExpired()) {
        _state = _State::Reading;
    }
}

bool ParticleSensor::_cycleReading() {
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
