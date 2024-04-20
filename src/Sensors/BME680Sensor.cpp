#include "Sensors/BME680Sensor.h"

#include "Diagnostics/Logging.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

BME680Sensor::BME680Sensor()
    : _sensor()
    , _initialised([this]{return _init();}, TimeSpan::fromSeconds(5))
    , _tempC()
    , _humidity()
    , _pressure()
    , _gas()
    , _measured(false)
{}

void BME680Sensor::setup() {}

void BME680Sensor::loop()
{
    if (_initialised) {
        _readMeasurements();
    }
}

float BME680Sensor::readTemperature() {
    return _tempC;
}

float BME680Sensor::readHumidity() {
    return _humidity;
}

float BME680Sensor::readAirPressure() {
    return _pressure;
}

float BME680Sensor::readGasLevel() {
    return _gas;
}

bool BME680Sensor::_init()
{
    bool ok(_sensor.begin());

    if (ok) {
        // Set up oversampling and filter initialization
        _sensor.setTemperatureOversampling(BME680_OS_8X);
        _sensor.setHumidityOversampling(BME680_OS_2X);
        _sensor.setPressureOversampling(BME680_OS_4X);
        _sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);
        _sensor.setGasHeater(320, 150); // 320*C for 150 ms        
        Log.infoln("BME680 connected.");
        _sensor.beginReading();
    }

    else {
        Log.errorln("BME680 not connected!");
    }

    return ok;    
}

void BME680Sensor::_readMeasurements() {
    if (_sensor.remainingReadingMillis() <= 0) { 
        _sensor.endReading();
        Log.verboseln("BME680 read.");
        _tempC =_sensor.readTemperature();
        _humidity = _sensor.readHumidity();
        _pressure = _sensor.readPressure();
        _gas = _sensor.readGas();
        _measured = true;
        _sensor.beginReading();
    }
}
