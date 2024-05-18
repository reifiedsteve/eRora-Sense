#pragma once

#include <LiquidCrystal_I2C.h>

#include "Displays/CharacterMatrix.h"

class CharacterMatrix1602 : public CharacterMatrix
{
public:

    static const byte bitmapAirflow[];
    static const byte bitmapThermometer[];
    static const byte bitmapDegrees[];
    static const byte bitmapSubscriptTwo[];
    static const byte bitmapSuperscriptThree[];
    static const byte bitmapWaterDrop[];
    static const byte bitmapHalf[];
    static const byte bitmapVeryHappy[];
    static const byte bitmapHappy[];
    static const byte bitmapAmbivalent[];
    static const byte bitmapSad[];
    static const byte bitmapVerySad[];
    static const byte bitmapScared[];

    explicit CharacterMatrix1602(uint8_t i2cAddr = 0x27, uint8_t cols = 16, uint8_t rows = 2);

    void defineCustomChar(size_t index, byte bitmap[]);

    void on();
    void off();

    void setup();

protected:

    void _writeText(int y, int x, const std::string& str);
    void _writeChar(int y, int x, char ch);
    void _refresh() override;

private:

    static char _mapCharacter(char ch);
    static bool _inRange(int value, int low, int high);

    LiquidCrystal_I2C _lcd;
};
