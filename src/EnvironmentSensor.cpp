#include "EnvironmentSensor.h"

bool EnvironmentSensor::setup()
{
    bool ok(_sensor.begin());

    if (ok) {
        // Set up oversampling and filter initialization
        _sensor.setTemperatureOversampling(BME680_OS_8X);
        _sensor.setHumidityOversampling(BME680_OS_2X);
        _sensor.setPressureOversampling(BME680_OS_4X);
        _sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);
        _sensor.setGasHeater(320, 150); // 320*C for 150 ms        
    }

    return ok;
}
