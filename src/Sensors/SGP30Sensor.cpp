#include "Sensors/SGP30Sensor.h"
#include "Chronos/TimeSpan.h"
#include "Diagnostics/Logging.h"

SGP30Sensor::SGP30Sensor(TwoWire& i2cBus /*, const UserSettings& userSettings */)
  : _i2cBus(i2cBus)
  , _sensor()
  , _restoredCalibration(false)
  , _timer(Stopwatch::State::Running)
  , _timeRead(0)
  , _timeReadRaw(0)
{}

void SGP30Sensor::setup() {
    _sensor.begin(&_i2cBus);
    _restoredCalibration = _loadBaseline();
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
    return _sensor.TVOC;
}

uint16_t SGP30Sensor::readCO2() { // Actuallu eCO2 rather than CO2 specifically.
    return _sensor.eCO2;
}

uint16_t SGP30Sensor::readHydrogen() {
    return _sensor.rawH2;
}

uint16_t SGP30Sensor::readEthenol() {
    return _sensor.rawEthanol;
}
    
bool SGP30Sensor::calibrate()
{
    // Sensor needs calibrating...
    // https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9_Gas_Sensors/Sensirion_Gas_Sensors_SGP30_Driver-Integration-Guide_SW_I2C.pdf

    // Readings are only valid after a 12-hour burning period,
    // (TODO: and within 7-days of being baselined).

    bool written(false);

    if (_isSteady())
    {
        uint16_t tvoc, co2;

        if (_sensor.getIAQBaseline(&tvoc, &co2)) {
            written =  (_saveFunc && _saveFunc(tvoc, co2));
        }
    }

    return written;
}

bool SGP30Sensor::isCalibrated() const {
    // TODO: calibration only valid for up to 7 days. Need to persist timestamp of calibration.
    return _restoredCalibration || _isSteady();
}

uint16_t SGP30Sensor::readBaselineTVOC() {
    uint16_t tvoc, co2;
    _sensor.getIAQBaseline(&tvoc, &co2);
    return tvoc;
}

uint16_t SGP30Sensor::readBaselineCO2() { // Actuallu eCO2 rather than CO2 specifically.
    uint16_t tvoc, co2;
    _sensor.getIAQBaseline(&tvoc, &co2);
    return co2;
}

bool SGP30Sensor::_saveBaseline() {
    return _saveFunc && _saveFunc(_sensor.TVOC, _sensor.eCO2);
}

bool SGP30Sensor::_loadBaseline() {
    bool loaded(false);
    uint16_t baselineTVOC, baselineCO2;
    if (_loadFunc && _loadFunc(baselineTVOC, baselineCO2)) {
        // TODO: store last time baseline was set. (> 7-days is stale).
        _sensor.setIAQBaseline(baselineTVOC, baselineCO2);
        loaded = true;
        Log.infoln("SPG30ensor: restored baseline TVOC to %d, baseline CO2 to %d.", baselineTVOC, baselineCO2);
    }
    return loaded;
}

bool SGP30Sensor::_isSteady() const {
    return _timer.elapsed() > TimeSpan::fromHours(12).millis();    
}
