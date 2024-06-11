#pragma once

#include "Sensors/TemperatureObserver.h"
#include "Sensors/HumidityObserver.h"
#include "Sensors/AirPressureObserver.h"
#include "Sensors/TVOCObserver.h"
#include "Sensors/CO2Observer.h"
#include "Sensors/IAQObserver.h"
#include "Sensors/PMObserver.h"

class SmartSensorMeasurementObserver
    : public TemperatureObserver
    , public HumidityObserver
    , public AirPressureObserver
    , public TVOCObserver
    , public CO2Observer
    , public IAQObserver
    , public PMObserver
    // , public ParticleObserver
{
public:

    explicit SmartSensorMeasurementObserver(const char* observerName)
      : _observerName(observerName)
    {}

     inline const char* measurementObserverName() const {
        return _observerName;
    }

private:

    const char* _observerName;
};


