#pragma once

#include <sstream>

#include "Sensors/TemperatureObserver.h"
#include "Sensors/HumidityObserver.h"
#include "Sensors/TVOCObserver.h"
#include "Sensors/CO2Observer.h"
#include "Sensors/ParticleObserver.h"

#include "Displays/SD1602/CharacterMatric1602.h"

#include "Chronos/CountdownTimer.h"

#if 0

class EnviromentDisplay 
  : public TemperatureObserver
  , public HumidityObserver
  , public TVOCObserver
  , public CO2Observer
  , public ParticleObserver
{
public:

    enum class Page {
        Overview,
        Page2,
        Page3,

        MaxPages
    };

    EnvironmentDisplay() 
      : _lcd()
      , _timer(5000, CountdownTimer::State::Running)
    {
        // _lcd.createChar(0, tempIcon)     // USe thermometer
        // _lcd.createChar(1, humidityIcon) // USe droplets.
        //// _lcd.createChar(2, tvocIcon)     // Use "vapour rising"
        // _lcd.createChar(3, subscript2)   // Use for CO2"
        //// _lcd.createChar(4, pmIcon)       // Use exclamation?
        // _lcd.createChar(5, fanIcon);     // Use a fan!
        // _lcd.createChar(6, microChar);   // Use "u" thingy.
        // _lcd.createChar(7, super3);      // Use superscript 3 condensed.
    }

    void setup() {

    }

    void showPage(Page page) {
        _showPage(page);
    }

    void showNextPage() {

    }

    void showPrevPage() {

    }

    void setDisplayTime(const TimeSpan& period) {

    }   

    void onTemperature(float tempC) override {
        _tempC = tempC;
    }

    void onHumidity(float relHumidity) override {
        _relHumidity = relHumidity;
    }

    void onTVOC(uint16_t ppm) override {
        _tvoc = ppm;
    }

    void onCO2(uint16_t ppm) override {
        _co2 = ppm;
    }

    void onParticleReading(uint16_t pm1, uint16_t pm25, uint16_t pm10) override {
        _pm1 = pm1;
        _pm25 = pm25;
        _pm10 = pm10;
    }

    void loop() {
        // If no page selection recently, then revert back to the overview page.
        if (_timer.hasExpired() && (_page != Page::Overview)) {
            _showPage(Page::Overview);
        }
    }

private:

    void _showPage(Page page)
    {
        switch (page) {
            case Page::Overview:
                _showOverview();
                break;
            case Page::Page2:
                _showPage2();
                break;
            case Page::Page3:
                _showPage3();
                break;
        }

        _timer.restart();
        _page = page;
    }

    void _showOverview() {
        // Degree symbol is 253 on these displays.
        _lcd.writeLine(0, _makeOverviewLine0());
        _lcd.writeLine(1, _makeOverviewLine1());
        _lcd.show();
    }    

    void _showPPage2() {

    }

    void _showPage3() {

    }

    std::string _makeOverviewLine1() {
        std::stringstream ss;
        ss << (char)0 << std::setw(3) << _tempC << (char)253 << "C";
        ss << " ";
        ss << (char)1 << std::setw(2) << _relHumidity << "%";
        ss << " " << " ";
        ss << (char)5 << std::setw(2) << 
        return ss.str();
    }

    std::string _makeOverviewLine2() {
        std::string ugm3("xxxx")
        std::string ppm("ppm");
        std::stringstream ss;
        ss << (char)2 << _tvoc << ppm;
        ss << " ";
        ss << (char)3 << _co2 << ppm;
        ss << " ";
        ss << "PM-1: " << _pm1 << ugm3;
        ss << " ";
        ss << "PM-2.5: " << _pm25 << ugm3;
        ss << " ";
        ss << "PM-10: " << _pm10 << ugm3;
        return ss.str();
    }

    CharacterMatrix1602 _lcd;
    Page _page;

    float _tempC, _relHumidity;
    uint16_t _tvoc, _co2, _pm1, _pm25, _pm10;

    CountdownTimer _timer;
};

#endif
