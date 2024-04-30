#include "Displays/SD1602/CharacterMatrix1602.h"

CharacterMatrix1602::CharacterMatrix1602(uint8_t i2cAddr, uint8_t cols, uint8_t rows)
    : CharacterMatrix(cols, rows)
    , _lcd(i2cAddr, cols, rows)
{}

void CharacterMatrix1602::setup() {
    _lcd.init();
    _lcd.clear();
    _lcd.backlight();
}

void CharacterMatrix1602::defineCustomChar(size_t index, byte bitmap[]) {
    _lcd.createChar(index, (byte*)bitmap);
}

void CharacterMatrix1602::on() {
    _lcd.backlight();
    _lcd.display();
}

void CharacterMatrix1602::off() {
    _lcd.noDisplay();
    _lcd.noBacklight();
}

void CharacterMatrix1602::_writeText(int y, int x, const std::string& str) {
    if ((y < height()) && (x < width())) {
        //Log.verboseln("CharacterMatrix1602: at y=%d, x=%d, printing \"%s\"", y, x, str.c_str());
        _lcd.setCursor(x, y);
        #if 1
            #if 1
                size_t len(str.size());
                //Log.verboseln("CharacterMatrix1602: len is %d.", len);
                for (int i = 0; i < len; ++i) {
                    char ch(str[i]);
                    char mapCh(_mapCharacter(ch));
                    //Log.verboseln("CharacterMatrix1602: printing %d (from %d).", (int)mapCh, (int)ch);
                    _lcd.print(mapCh);
                }
            #else
                for(char&ch : str) {
                    char mapCh(_mapCharacter(ch));
                    //Log.verboseln("CharacterMatrix1602: printing %d (from %d).", (int)mapCh, (int)ch);
                    _lcd.print(mapCh);
                }
            #endif
        #else
        _lcd.print(str.c_str());
        #endif
    }
}

void CharacterMatrix1602::_writeChar(int y, int x, char ch) {
    if ((y < height()) && (x < width())) {
        _lcd.setCursor(x, y);
        _lcd.write(ch);
    }
}

void CharacterMatrix1602::_refresh() {
    // Anything to do here?
}

char CharacterMatrix1602::_mapCharacter(char ch) {
    char retCh(ch);
    if (_inRange(retCh, 128, 135)) {
        retCh -= 128;
    }
    return retCh;
}

bool CharacterMatrix1602::_inRange(int value, int low, int high) {
    return ((low <= value) && (value <= high));
}

const byte CharacterMatrix1602::bitmapAirflow[] = {
    0b00000,
    0b01000,
    0b10101,
    0b00010,
    0b01000,
    0b10101,
    0b00010,
    0b00000
};

const byte CharacterMatrix1602::bitmapThermometer[] = {
    0b00100,
    0b01010,
    0b01010,
    0b01110,
    0b11111,
    0b11111,
    0b01110,
    0b00000
};

const byte CharacterMatrix1602::bitmapDegrees[] = {
    0b00100,
    0b01010,
    0b00100,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

const byte CharacterMatrix1602::bitmapSubscriptTwo[] = {
    0b00000,
    0b00000,
    0b11000,
    0b00100,
    0b11000,
    0b10000,
    0b11100,
    0b00000,
};

const byte CharacterMatrix1602::bitmapSuperscriptThree[] = {
    0b00000,
    0b11000,
    0b00100,
    0b11000,
    0b00100,
    0b11000,
    0b00000,
    0b00000,
};

const byte CharacterMatrix1602::bitmapWaterDrop[] = {
    0b00100,
    0b00100,
    0b01110,
    0b11111,
    0b11101,
    0b11111,
    0b01110,
    0b00000
};

const byte CharacterMatrix1602::bitmapHalf[] = {
    0b10000,
    0b10000,
    0b10110,
    0b10001,
    0b00110,
    0b00100,
    0b00111,
    0b00000,
};

const byte CharacterMatrix1602::bitmapHappy[] = {
    0b00000,
    0b00000,
    0b01010,
    0b00000,
    0b10001,
    0b01110,
    0b00000,
    0b00000,
};

const byte CharacterMatrix1602::bitmapAmbivalent[] = {
    0b00000,
    0b00000,
    0b01010,
    0b00000,
    0b11111,
    0b00000,
    0b00000,
    0b00000,
};

const byte CharacterMatrix1602::bitmapSad[] = {
    0b00000,
    0b00000,
    0b01010,
    0b00000,
    0b01110,
    0b10001,
    0b00000,
    0b00000,
};
