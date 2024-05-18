#include "Sensors/SGP30Sensor.h"
#include "Chronos/TimeSpan.h"
#include "Diagnostics/Logging.h"

SGP30Sensor::SGP30Sensor(TwoWire& i2cBus)
  : _i2cBus(i2cBus)
  , _connected(false)
  , _saveFunc()
  , _loadFunc()
  , _sensor()
  , _restoredCalibration(false)
  , _timer(Stopwatch::State::Running)
  , _timeRead(0)
  , _timeReadRaw(0)
{}

void SGP30Sensor::setup() {
    _connected = _sensor.begin(&_i2cBus);
    if (_connected) {
        _restoredCalibration = _restoreBaseline();
    }
}

void SGP30Sensor::loop()
{
    if (_sensor.IAQmeasure()) {
        _timeRead = _timer.elapsed();
    }

    if (_sensor.IAQmeasureRaw()) {
        _timeReadRaw = _timer.elapsed();
    }
}

float SGP30Sensor::readTVOC() {
    return _sensor.TVOC;
}

float SGP30Sensor::readCO2() { // Actuallu eCO2 rather than CO2 specifically.
    return _sensor.eCO2;
}

uint16_t SGP30Sensor::readHydrogen() {
    return _sensor.rawH2;
}

uint16_t SGP30Sensor::readEthenol() {
    return _sensor.rawEthanol;
}

SGP30Sensor::CalibrationResult SGP30Sensor::calibrate()
{
    // Sensor needs calibrating...
    // https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9_Gas_Sensors/Sensirion_Gas_Sensors_SGP30_Driver-Integration-Guide_SW_I2C.pdf

    // Readings are only valid after a 12-hour burn-in period from cold.
    // (TODO: and within 7-days of being baselined, upon which must be recalibrated).

    CalibrationResult result(CalibrationResult::OK);

    if (!_isSteady()) {
        result = CalibrationResult::TooEarly;
    }
    
    else
    {
        uint16_t tvoc, co2;

        if (_sensor.getIAQBaseline(&tvoc, &co2))
        {
            if (!_saveFunc) {
                result = CalibrationResult::NoSaveFunc;
            }

            else if (!_saveFunc(tvoc, co2)) {
                result = CalibrationResult::SaveFailed;
            }
        }
    }

    return result;
}

bool SGP30Sensor::isConnected() const {
    return _connected;
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

bool SGP30Sensor::_restoreBaseline() {
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

/// @brief Returns whether the sensor has been on long enough to have established a baseline reading.
/// @return If a baseline has been established, then return true; otherwise false.
bool SGP30Sensor::_isSteady() const {
    return _timer.elapsed() > TimeSpan::fromHours(12).millis();    
}

/// @brief Calculate approx value for absolute humidity from relative humidity and temperature,
/// @param temperature The temperatuer in C.
/// @param humidity The relative humidity as a percentage.
/// @return The absolute humidity in mg/m^3.
uint32_t SGP30Sensor::_calculateAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}
