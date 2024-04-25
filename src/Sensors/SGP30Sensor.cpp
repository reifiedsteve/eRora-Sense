#include "Sensors/SGP30Sensor.h"

SGP30Sensor::SGP30Sensor(TwoWire& i2cBus)
  : _i2cBus(i2cBus)
  , _sensor()
  , _timer(Stopwatch::State::Running)
  , _tvoc(0)
  , _eco2(0)
  , _timeRead(0)
  , _hydrogen(0)
  , _ethenol(0)
  , _timeReadRaw(0)
{}

void SGP30Sensor::setup() {
    _sensor.begin(&_i2cBus);
}

void SGP30Sensor::loop()
{
    if (_sensor.IAQmeasure()) {
        _tvoc = _sensor.TVOC;
        _eco2 = _sensor.eCO2;
        _timeRead = _timer.elapsed();
    }

    if (_sensor.IAQmeasureRaw()) {
        _hydrogen = _sensor.rawH2;
        _ethenol = _sensor.rawEthanol;
        _timeReadRaw = _timer.elapsed();
    }
            
}

uint16_t SGP30Sensor::readTVOC() {
    return _tvoc;
}

uint16_t SGP30Sensor::readCO2() { // Actuallu eCO2 rather than CO2 specifically.
    return _eco2;
}

uint16_t SGP30Sensor::readHydrogen() {
    return _hydrogen;
}

uint16_t SGP30Sensor::readEthenol() {
    return _ethenol;
}
    

bool SGP30Sensor::calibrateBaseline()
{
    bool done(_timeRead > 0); // i.e. have we taken a reading yet.

    if (done) {
        _sensor.setIAQBaseline(_tvoc, _eco2);
    }

    return done;
}
