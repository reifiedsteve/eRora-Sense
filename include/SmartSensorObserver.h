#pragma once

#include "SmartSensorMeasurementObserver.h"
#include "SmartSensorStateObserver.h"

class SmartSensorObserver
  : public SmartSensorStateObserver
  , public SmartSensorMeasurementObserver
{
public:

    SmartSensorObserver(const char* observerName)
      : SmartSensorMeasurementObserver(observerName) 
      , SmartSensorStateObserver(observerName) 
      , _observerName(observerName)
    {}

     inline const char* observerName() const {
        return _observerName;
    }

private:

    const char* _observerName;
};
