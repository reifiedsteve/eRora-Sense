#pragma once

#include "Sensors/OccupancySensor.h"

#include <Arduino.h>

class LD2410Sensor : public OccupancySensor
{
public:

    explicit LD2410Sensor(int pinNo);

    void setup();
    void loop();

    bool readOccupancy() override;

private:

    int _pinNo;
    bool _present;
};
