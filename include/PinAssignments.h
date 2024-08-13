#pragma once

// Useful pin-out diagram for ESP32. These nimbers refer to the pins designated with "GPIO" labels.
//s https://www.etechnophiles.com/wp-content/uploads/2021/03/esp32-Board-with-30-pins-Pinout.

struct PinAssignments
{
    // Assignment of GPIO pin numbers.

    static const int OnboardLED = 2;
    static const int OnboardBootButton = 0;
    
    static const int Button1 = 12;   // Pwr.   12
    static const int Button2 = 14;   // Mode.  14
    static const int Button3 = 27;   // Fan-   27   (was 25).
    static const int Button4 = 26;   // Fan+   26
    
    static const int PMSerialRX = 16;
    static const int PMSerialTX = 17;

    static const int FanPWM = 5;    
    static const int FanPowerControl = 23; // e.g. to control a MOSFET/relay.

    static const int CabinetLightsDataPin = 25;
    
    // Al I2C devices use Arduino TwoWire abstraction, using the
    // default SDA,SCL pins i.e. SDA is GPIO-21, SCL is GPIO-22
    // A 1602 LCD di    msplay is attached via I2C, as is the BME680 sensor.
};




