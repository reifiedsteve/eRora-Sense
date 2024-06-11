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
    // _pms.wake();
    //_pms.sleep();
}

void PMS7003ParticleSensor::loop() {
    _available = _available || _cycle();
}

bool PMS7003ParticleSensor::available() {
    return true;
    // return _available;
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
    // _pms.wake();
    _changeState(_State::Reading);
}

void PMS7003ParticleSensor::_cycleSleeping() {
    if (_timer.hasExpired()) {
        // _pms.wake();
        _changeState(_State::Reading);
    }
}

bool PMS7003ParticleSensor::_cycleReading()
{
    bool available(false);
    _pms.read();

    if (_pms)
    {
        if (_pms.has_particulate_matter()) {
            Log.verboseln("PMS7003ParticleSensor: has PM data.");
            Log.infoln("Particles: PM1.0 %d, PM2.5 %d, PM10.0 %d", _pms.pm01, _pms.pm25, _pms.pm10);
            _measurements.pm01 = _pms.pm01;
            _measurements.pm25 = _pms.pm25;
            _measurements.pm10 = _pms.pm10;
            _timer.restart();
            _changeState(_State::Sleeping);
            //_pms.sleep();
            available = true;
        }
        
        else {
            Log.errorln("PMS7003 returnd status of %d.", _pms.status);
        }

        // TODO: should we only change back to Sleeping upon a successful read?
        //_timer.restart();
        //_changeState(_State::Sleeping);
    }

    else
    { // something went wrong
        switch (_pms.status)
        {
            case _pms.OK: // should never come here
                Serial.println("PMS statis is OK");
                break;     // included to compile without warnings

            case _pms.ERROR_TIMEOUT:
                Serial.println(F(PMS_ERROR_TIMEOUT));
                break;

            case _pms.ERROR_MSG_UNKNOWN:
                Serial.println(F(PMS_ERROR_MSG_UNKNOWN));
                break; 

            case _pms.ERROR_MSG_HEADER:
                Serial.println(F(PMS_ERROR_MSG_HEADER));
                break;

            case _pms.ERROR_MSG_BODY:
                Serial.println(F(PMS_ERROR_MSG_BODY));
                break;

            case _pms.ERROR_MSG_START:
                Serial.println(F(PMS_ERROR_MSG_START));
                break;

            case _pms.ERROR_MSG_LENGTH:
                Serial.println(F(PMS_ERROR_MSG_LENGTH));
                break;

            case _pms.ERROR_MSG_CKSUM:
                Serial.println(F(PMS_ERROR_MSG_CKSUM));
                break;

            case _pms.ERROR_PMS_TYPE:
                Serial.println(F(PMS_ERROR_PMS_TYPE));
                break;
        }
    }

    return available;
}


void PMS7003ParticleSensor::_changeState(_State newState) {
    if (_state != newState) {
        Log.verboseln("PMS7003ParticleSensor: changing state from %s to %s.", _toStr(_state).c_str(), _toStr(newState).c_str());
        _state = newState;
    }
}

std::string PMS7003ParticleSensor::_toStr(_State state)
{
    std::string str("?");

    switch (state)
    {
        case _State::Init:
            str = "Init";
            break;

        case _State::Sleeping:
            str = "Sleeping";
            break;

        case _State::Reading:
            str = "Reading";
            break;
    }

    return str;
}
