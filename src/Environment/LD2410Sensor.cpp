#include "Environment/LD2410Sensor.h"

#include <Arduino.h>

LD2410Sensor::LD2410Sensor(int pinNo) 
    : _pinNo(pinNo) 
    , _present(false)
{}

void LD2410Sensor::setup() {
    pinMode(_pinNo, INPUT);
}

void LD2410Sensor::loop() {
    _present = (digitalRead(_pinNo) == HIGH);
}

bool LD2410Sensor::readOccupancy() {
    return _present;
}
