#pragma once

#include "Displays/SD1602/CharacterMatrix1602.h"

#include "SmartSensorObserver.h"
// #include "PWMFanObserver.h"

#include "Chronos/CountdownTimer.h"

class PanelDisplay : public SmartSensorObserver
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
    static const char faceChar = 7; // Polymorphic expressions.  :O  :'(  :(   :|   :)  :))

    static const char microChar = '\xE4';

    PanelDisplay();

    void selectNextPage();

    void setup();
    void loop();

    const char* name() const { return "LCD1602"; }
    
    void onSwitchOnOff(bool on) override;
    void onFanSpeed(int speed) override;
    void onCabinetBrightness(uint8_t brightness) override;
    void onCabinetColour(uint8_t hue, uint8_t sat) override;

    void onTemperature(float temperature) override;
    void onHumidity(float humidity) override;
    void onTVOC(float tvoc) override;
    void onCO2(float co2) override;
    void onAirPressure(float hPa) override;
    void onIAQAvailability(bool available) override; // IAQ, TVOC, CO2.
    void onIAQ(float iaq) override;

    void onPM01(uint16_t pm01) override;
    void onPM25(uint16_t pm25) override;
    void onPM10(uint16_t pm10) override;
    
    void onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) override;

private:

    static const size_t _width = 16;

    enum class _Mode {
        Overview,
        Details,
        Settings,
        About
    };

    enum class _Page {
        Welcome0,
        Welcome1,
        IAQ,
        TVOC,
        CO2,
        PM01,
        PM25,
        PM10,
        CalibrationState,
        Broken
    };

    enum class _Category {
        Good,
        Moderate,
        Sensitive,
        Unhealthy,
        VeryUnhealthy,
        Hazardous
    };

    enum class _State {
        Normal,
        StartNotification,
        Notification
    };
    
    void _defineCustomChars();

    void _execStateNormal();
    void _execStateStartNotification();
    void _execStateNotification();

    _Page _pageAfter(_Page page);
    void _showPage(_Page page);

    void _notify(const std::string& title, const std::string& message);
    
    std::string _makeTemperatureHumidityFanSpeedText();
    std::string _makeTVOCLine();
    std::string _makeCO2Line();
    std::string _makeIAQLine();
    std::string _makePMLine(const std::string& label, const uint16_t pmLevel);
    std::string _makeFanSpeedNotificationMessage(int fanSpeed);
    std::string _makeCalibrationStateText();

    static std::string _makeFaceIcon(_Category cat);
    static std::string _makeIAQDescription(int iaq);
    static std::string _makeIAQDescription(_Category cat);

    static _Category _determineIAQCategory(int iaq);
    static _Category _determineTVOCCategory(int iaq);
    static _Category _determineCO2Category(int co2);

    void _selectFaceIcon(_Category cat);
    static const byte* _getFaceIconBitmap(_Category cat);

    /*
    void _makeFaceHappy();
    void _makeFaceAmbivalent();
    void _makeFaceSad();
    */

    static std::string _fixLeft(const std::string& str, size_t len);
    static std::string _fixCentre(const std::string& str, size_t fixedLen);

    static char _customChar(char ch);

    Display _display;
    _Page _page;

    float _tempC, _relHumidity;
    float _tvoc, _co2;
    float _airPressure;

    bool _iaqAvailable;
    float _iaq;

    uint16_t _pm01, _pm25, _pm10;

    int _fanSpeed;
    
    CountdownTimer _timer;

    std::string _title, _message;
    CountdownTimer _notifyTimer;

    _State _state;
};
