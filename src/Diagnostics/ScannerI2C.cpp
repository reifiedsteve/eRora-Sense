 
// #### Uncomment and use where necessary. ####

#include <Arduino.h>
#include <Wire.h>
 
#define I2C_Freq 100000

// #define SDA_0 18
// #define SCL_0 19

#define SDA_0 21
#define SCL_0 22
 
TwoWire I2C_0 = TwoWire(0);
TwoWire I2C_1 = TwoWire(1);

# include "Diagnostics/ScannerI2C.h"

void ScannerI2C::setup()
{
    Serial.begin(115200);
    I2C_0.begin((int)SDA_0, (int)SCL_0, (uint32_t)I2C_Freq);
}

void ScannerI2C::loop() {
    _scan(I2C_0);
    // _scan(I2C_1);
}

void ScannerI2C::_scan(TwoWire& wire)
{
    byte error, address;
    int nDevices;

    Serial.println("Scanning...");
    nDevices = 0;

    for(address = 1; address < 127; address++ )
    {
        Serial.print("Checking ");
        Serial.print((int)address);
        Serial.print("\r");
        
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        wire.beginTransmission(address);
        error = wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address<16) {
                Serial.print("0");
            }
            Serial.print(address,HEX);
            Serial.println(" !!!");
            nDevices++;
        }

        else if (error==4) {
            Serial.print("Unknown error at address 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }    
    }

    if (nDevices == 0) {
        Serial.println("No I2C devices found.");
        Serial.println("");
    } else {
        Serial.println("done.           ");
        Serial.println("");
    }

    delay(5000);           // wait 5 seconds for next scan
}

