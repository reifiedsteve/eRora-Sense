#pragma once

// Useful pin-out diagram for ESP32. These nimbers refer to the pins designated with "GPIO" labels.
//s https://www.etechnophiles.com/wp-content/uploads/2021/03/esp32-Board-with-30-pins-Pinout.

struct PinAssignments
{
    // Assignment of GPIO pin numbers.

    static const int OnboardLED = 2;
    static const int OnboardBootButton = 0;
    
    static const int Button1 = 5;
    static const int Button2 = 17;
    static const int Button3 = 4;
    static const int Button4 = 16;
    
    static const int FanPWM = 14;

    // Al I2C devices use Arduino TwoWire abstraction, using the
    // default SDA,SCL pins i.e. SDA is GPIO-21, SCL is GPIO-22
};




