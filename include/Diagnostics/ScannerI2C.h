#pragma once

#include <Arduino.h>
#include <Wire.h>
   
class ScannerI2C
{
public:

    void setup();    
    void loop();

private:

    void _scan(TwoWire& wire);
};


