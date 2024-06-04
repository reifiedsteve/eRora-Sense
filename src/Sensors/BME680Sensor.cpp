#include "Sensors/BME680Sensor.h"

#include "Diagnostics/Logging.h"

// Useful info: technical datasheet for the Bodch BME680:
// https://www.bosch-sensortec.com/media/boschsensortec/downloads/handling_soldering_mounting_instructions/bst-bme680-hs000.pdf

static bsec_virtual_sensor_t sensorList[13] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
};

BME680Sensor::BME680Sensor(const TimeSpan& sampleInterval)
    : _sensor()
    // , _initialised([this]{return _init();}, TimeSpan::fromSeconds(5))
    , _initialised(false)
    , _timer(sampleInterval.millis())
    , _tempC()
    , _humidity()
    , _pressure()
    , _eCO2(0)
    , _tvoc(0)
    , _iaq(0)
    , _measured(false)
    , _tempOffset(0.0)
    , _humidityOffset(0.0)
{}

void BME680Sensor::setup() {
    _initialised = _init();
    _timer.start();
    /*
    if (_initialised) {
        _timer.start();
    }
    */
}

void BME680Sensor::loop()
{
    // Performing samples frequently reduces the lifetime of the sensor,
    // hence why we only sample periodically rather than flat-out.

    //if (_timer.hasExpired()) {     // <==-- No! Need to keep calling _readMeasurements() to keep sensor working.
        //Log.verboseln("Trying to read measurements...");
        if (_readMeasurements()) {
            _measured = true;
            //_timer.restart();
        }
    //}
}

bool BME680Sensor::connected() {
    return _initialised || _init();
}

/*
bool BME680Sensor::isTemperatureSensorReady() {
    return true;
}
*/

bool BME680Sensor::isTemperatureAvailable() {
    return _measured;
}

float BME680Sensor::readTemperature() {
    return _measured ? (_tempC + _tempOffset) : 0.0;
}

/*
bool BME680Sensor::isHumiditySensorReady() {
    return true;
}
*/

bool BME680Sensor::isHumidityAvailable() {
    return _measured;
}

float BME680Sensor::readHumidity() {
    return _measured ? (_humidity + _humidityOffset) : 0.0;
}

/*
bool BME680Sensor::isAirPressureSensorReady() {
    return true; // TODO: check this is appropriate.
}
*/

bool BME680Sensor::isAirPressureAvailable() {
    return _measured;
}

float BME680Sensor::readAirPressure() {
    return _pressure / 100.0; // Return hPa, not Pa.
}

/*
bool BME680Sensor::isTVOCSensorReady() {
    return _sensor.runInStatus > 0.0;
}
*/

bool BME680Sensor::isTVOCAvailable() {
    return (_sensor.runInStatus > 0.0) && _measured;
}

float BME680Sensor::readTVOC() {
    return _tvoc;
}

/*
bool BME680Sensor::isCO2SensorReady() {
    return _sensor.runInStatus > 0.0;
}
*/

bool BME680Sensor::isCO2Available() {
    return (_sensor.runInStatus > 0.0) && _measured;
}

float BME680Sensor::readCO2() {
    return _eCO2;
}

/***
bool BME680Sensor::isIAQSensorReady() {
    return _sensor.runInStatus > 0.0;
}
***/

bool BME680Sensor::isIAQAvailable() {
    return (_sensor.runInStatus > 0.0) && _measured;
}

float BME680Sensor::readIAQ() {
    return _iaq;
}

bool BME680Sensor::_init()
{
    _sensor.begin(BME68X_I2C_ADDR_LOW, Wire);
    _sensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);

    Log.infoln("BME6800: Version %d.%d.%d.%d", _sensor.version.major, _sensor.version.minor, _sensor.version.major_bugfix, _sensor.version.minor_bugfix);

    return true;
}

bool BME680Sensor::_readMeasurements()
{
    bool gotNewReadings(_sensor.run());

    // Log.verboseln("BME680 stable: %s.", String(_sensor.stabStatus));
    // Log.verboseln("BME680 run-in: %s.", String(_sensor.runInStatus));

    if (gotNewReadings)
    {
        /*
        Bsec& iaqSensor(_sensor);
        String output("BME680: ");
        output += ", " + String(iaqSensor.iaq);
        output += ", " + String(iaqSensor.iaqAccuracy);
        output += ", " + String(iaqSensor.staticIaq);
        output += ", " + String(iaqSensor.co2Equivalent);
        output += ", " + String(iaqSensor.breathVocEquivalent);
        output += ", " + String(iaqSensor.rawTemperature);
        output += ", " + String(iaqSensor.pressure);
        output += ", " + String(iaqSensor.rawHumidity);
        output += ", " + String(iaqSensor.gasResistance);
        output += ", " + String(iaqSensor.stabStatus);
        output += ", " + String(iaqSensor.runInStatus);
        output += ", " + String(iaqSensor.temperature);
        output += ", " + String(iaqSensor.humidity);
        output += ", " + String(iaqSensor.gasPercentage);
        Log.verboseln("%s", output.c_str());
        */

        _tempC = _sensor.temperature;
        _humidity = _sensor.humidity;
        _pressure = _sensor.pressure;

        // Log.verboseln("BSEC status: %d", (int)_sensor.bsecStatus);
        // Log.verboseln("BME68x status: %d", (int)_sensor.bme68xStatus);

        // Log.verboseln("Temperature: %d degC", (int)_tempC);
        // Log.verboseln("Humidity: %d pc", (int)_humidity);
        // Log.verboseln("Pressure: %d Pa", (int)_pressure);

        bool gasSensorReady(_sensor.runInStatus > 0.0); // 0.0 is not ready, 1.0 is ready.
        // Log.verboseln("Gas sensor ready: %s", gasSensorReady ? "Yes" : "No");

        if (gasSensorReady) 
        {
            _eCO2 = _sensor.co2Equivalent;
            _tvoc = _sensor.breathVocEquivalent;
            _iaq = _sensor.iaq;

            // Log.verboseln("eCO2: %d ppm", (int)_eCO2);
            // Log.verboseln("TVOC (ppb): %d", (int)_tvoc);
            // Log.verboseln("IAQ: %d", (int)_iaq);
        }
    }

    else {
        // Log.verboseln("BME680,run returns false");
    }

    return gotNewReadings;
}
