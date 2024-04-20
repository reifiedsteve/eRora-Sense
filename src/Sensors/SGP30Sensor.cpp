#include "Sensors/SGP30Sensor.h"

SGP30Sensor::SGP30Sensor()
  : _sensor()
  , _timer(Stopwatch::State::Running)
  , _tvoc(0)
  , _eco2(0)
  , _timeRead(0)
{}

void SGP30Sensor::setup() {
    _sensor.begin();
}

void SGP30Sensor::loop()
{
    if (_sensor.IAQmeasure()) {
        _tvoc = _sensor.TVOC;
        _eco2 = _sensor.eCO2;
        _timeRead = _timer.elapsed();
    }
}

uint16_t SGP30Sensor::readTVOC() {
    return _tvoc;
}

uint16_t SGP30Sensor::readCO2() { // Actuallu eCO2 rather than CO2 specifically.
    return _eco2;
}

bool SGP30Sensor::calibrateBaseline()
{
    bool done(_timeRead > 0);

    if (done) {
        _sensor.setIAQBaseline(_tvoc, _eco2);
    }

    return done;
}
