#pragma once

#include "Displays/SD1602/CharacterMatrix1602.h"

#include "Sensors/TemperatureObserver.h"
#include "Sensors/HumidityObserver.h"
#include "Sensors/TVOCObserver.h"
#include "Sensors/CO2Observer.h"
#include "Sensors/ParticleObserver.h"
// #include "PWMFanObserver.h"

#include "Chronos/CountdownTimer.h"

class PanelDisplay
  : public TemperatureObserver
  , public HumidityObserver
  , public TVOCObserver
  , public CO2Observer
  , public ParticleObserver
{
public:

    typedef CharacterMatrix1602 Display;

    // The D1602 allows a maximum of eight custom characters,
    // so we must choose wisely.

    static const char thermometerChar = 0; // aka, 128 (as // Cannot embed 0 in a C/C++ string).
    static const char waterDropChar = 1;
    static const char airflowChar = 2;
    static const char degreesChar = 3;
    static const char subscriptTwoChar = 4;
    static const char superscriptThreeChar = 5;
    static const char halfChar = 6;
    static const char faceChar = 7; // Polymorphic expressions. :(  :|  :)
    // static const char superscriptThreeChar = x;

    static const char microChar = '\xE4';

    PanelDisplay();

    void selectNextPage();

    void setup();
    void loop();

    void onTemperature(float temperature) override;
    void onHumidity(float humidity) override;
    void onTVOC(uint16_t tvoc) override;
    void onCO2(uint16_t co2) override;
    void onParticleReading(uint16_t pm01, uint16_t pm25, uint16_t pm10);
    
private:

    enum class _Page {
        Welcome1,
        Welcome2,
        TVOC,
        CO2,
        PM01,
        PM25,
        PM10,
        Broken
    };

    void _defineCustomChars();

    _Page _pageAfter(_Page page);
    void _showPage(_Page page);

    std::string _makeTemperatureHumidityFanSpeedText();
    std::string _makeTVOCLine();
    std::string _makeCO2Line();
    std::string _makePMLine(const std::string& label, const uint16_t pmLevel);

    void _makeFaceHappy();
    void _makeFaceAmbivalent();
    void _makeFaceSad();
    
    static std::string _fixTo(const std::string& str, size_t len);
    static char _customChar(char ch);

    Display _display;
    _Page _page;

    float _tempC, _relHumidity;;
    uint16_t _tvoc, _co2;
    uint16_t _pm01, _pm25, _pm10;

    CountdownTimer _timer;
};
