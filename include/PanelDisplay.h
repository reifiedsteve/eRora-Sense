#if 0

#pragma once

#include "Displays/SD1602/CharacterMatrix1602.h"

class PanelDisplay : public EnvironmentObserver
{
    typedef CharacterMatrix1602 Display;

    static const char happyChar = 128; // Cannot embed 0 in a C/C++ string.
    static const char happyChar = 1;
    static const char happyChar = 2;
    static const char happyChar = 3;
    static const char happyChar = 4;
    static const char happyChar = 5;
    static const char happyChar = 7;
    static const char happyChar = 7;

    PanelDisplay() : EnvironmentObserver() 
    {
        _display.defineCustomChar(0, (byte*)Display::bitmapHappy);
        _display.defineCustomChar(1, (byte*)Display::bitmapAirflow);   
        _display.defineCustomChar(2, (byte*)Display::bitmapThermometer);   
        _display.defineCustomChar(3, (byte*)Display::bitmapDegrees);   
        _display.defineCustomChar(4, (byte*)Display::bitmapSubscriptTwo);   
        _display.defineCustomChar(5, (byte*)Display::bitmapSuperscriptThree);   
        _display.defineCustomChar(6, (byte*)Display::bitmapWaterDrop);   
        _display.defineCustomChar(7, (byte*)Display::bitmapSad);

    }

private:

    Display _display;
};

#endif
