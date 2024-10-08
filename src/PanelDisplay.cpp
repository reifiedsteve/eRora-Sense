#include <sstream>
#include <iomanip>

#include <cmath>

#include "PanelDisplay.h"

PanelDisplay::PanelDisplay() 
    : SmartSensorObserver("panel-display")
    , _display()
    , _page(_Page::Welcome0)
    , _deviceState()
    , _timer(5000)
    , _title()
    , _message()
    , _notifyTimer(5000)
    , _displayState(_DisplayState::Normal)
{}

void PanelDisplay::selectNextPage() {
    _page = _pageAfter(_page);
    _timer.restart();
}

void PanelDisplay::setup() {
    _timer.start();
    _display.setup();
    _defineCustomChars();
}

void PanelDisplay::loop()
{
    switch (_displayState)
    {
        case _DisplayState::Normal:
            _execStateNormal();
            break;

        case _DisplayState::StartNotification:
            _execStateStartNotification();
            break;

        case _DisplayState::Notification:
            _execStateNotification();
            break;

        default:
            _displayState = _DisplayState::Normal;
            break;
    }

    // _showPage(_page); //  Doing this here is useful for real-time updates - e.g. for fan speed.
}

void PanelDisplay::onSwitchOnOff(bool on) {
    _deviceState.power = on;
    _notify("eRora Sensor", std::string("Power ") + (on ? "on" : "off"));
    if (on) _display.on(); else _display.off();
}

void PanelDisplay::onFanSpeed(int speed) {
    _deviceState.fanSpeed = speed;
    _notify("Fan Speed", _makeFanSpeedNotificationMessage(_deviceState.fanSpeed));
}

void PanelDisplay::onCabinetBrightness(uint8_t brightness) {

}

void PanelDisplay::onCabinetColour(uint8_t hue, uint8_t sat) {
    
}

void PanelDisplay::onCabinetInspectionLightOnOff(bool on) {
    if (on) {
        _notify("Inspection", "Light On");
    } 

}
void PanelDisplay::onTemperature(float temperature) {
    _deviceState.temperature = temperature;
}

void PanelDisplay::onHumidity(float humidity){
    _deviceState.relHumidity = humidity;
}

void PanelDisplay::onTVOC(float tvoc) {
    _deviceState.tvoc = tvoc;
}

void PanelDisplay::onCO2(float co2) {
    _deviceState.co2 = co2;
}

void PanelDisplay::onAirPressure(float hPa) {
    _deviceState.airPressure = hPa;
}

void PanelDisplay::onIAQAvailability(bool available) {
    _deviceState.sensorReady = available;
}

void PanelDisplay::onIAQ(float iaq) {
    _deviceState.iaq = iaq;
}

void PanelDisplay::onPM01(uint16_t pm01) {
    _deviceState.pm01 = pm01;
}

void PanelDisplay::onPM25(uint16_t pm25) {
    _deviceState.pm25 = pm25;
}

void PanelDisplay::onPM10(uint16_t pm10) {
    _deviceState.pm10 = pm10;
}

void PanelDisplay::onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) {
    // Probably not of interest in regard to showing on the LCD panel.
}

void PanelDisplay::_defineCustomChars() {
    _display.defineCustomChar(thermometerChar, const_cast<byte*>(Display::bitmapThermometer));
    _display.defineCustomChar(waterDropChar, const_cast<byte*>(Display::bitmapWaterDrop));   
    _display.defineCustomChar(airflowChar, const_cast<byte*>(Display::bitmapAirflow));   
    _display.defineCustomChar(degreesChar, const_cast<byte*>(Display::bitmapDegrees));   
    _display.defineCustomChar(subscriptTwoChar, const_cast<byte*>(Display::bitmapSubscriptTwo));   
    _display.defineCustomChar(superscriptThreeChar, const_cast<byte*>(Display::bitmapSuperscriptThree));   
    _display.defineCustomChar(halfChar, const_cast<byte*>(Display::bitmapHalf)); 
    _display.defineCustomChar(faceChar, const_cast<byte*>(Display::bitmapAmbivalent)); 
    // _display.defineCustomChar(superscriptThreeChar, const_cast<byte*>(Display::bitmapSuperscriptThree));   
}

void PanelDisplay::_execStateNormal()
{
    bool refresh(false);

    if (_timer.hasExpired()) {
        _page = _pageAfter(_page);
        refresh = true;
        _timer.restart();
    }
    
    if (refresh) {
        _showPage(_page);
    }
}

void PanelDisplay::_execStateStartNotification()
{
    _display.writeLine(0, _fixCentre(_title, _width));
    _display.writeLine(1, _fixCentre(_message, _width));
    _display.show();
    
    _notifyTimer.restart();
    _displayState = _DisplayState::Notification;
}

void PanelDisplay::_execStateNotification()
{
    if (_notifyTimer.hasExpired()) {
        _page = _pageAfter(_page);
        _showPage(_page);
        _timer.restart();
        _displayState = _DisplayState::Normal;
    }
}

PanelDisplay::_Page PanelDisplay::_pageAfter(_Page page)
{
    _Page nextPage(_Page::Welcome1);

    switch (page) 
    {
        case _Page::Welcome0:
            nextPage = _Page::Welcome1;
            break;

        case _Page::Welcome1:
            nextPage = _Page::IAQ;
            break;

        case _Page::IAQ:
            nextPage = _Page::TVOC;
            break;

        case _Page::TVOC:
            nextPage = _Page::CO2;
            break;

        case _Page::CO2:
            // nextPage = _Page::PM01;
            nextPage = _Page::IAQ;
            break;

        case _Page::PM01:
            nextPage = _Page::PM25;
            break;

        case _Page::PM25:
            nextPage = _Page::PM10;
            break;

        case _Page::PM10:
            nextPage = _Page::IAQ;
            break;

        default:
            break;
    }

    return nextPage;
}

void PanelDisplay::_showPage(_Page page)
{
    switch (page)
    {
        case _Page::Welcome0:
            _display.writeLine(0, "  eRora Sense   ");
            _display.writeLine(1, "  Air Quality   ");
            break;

        case _Page::Welcome1:
            _display.writeLine(0, "  eRora Sense   ");
            _display.writeLine(1, "PWM Fan Control ");
            break;

        case _Page::IAQ:
            _display.writeLine(0, "  Air Quality   ");
            _display.writeLine(1, _makeIAQLine());
            break;

        case _Page::TVOC:
            _display.writeLine(0, _makeTemperatureHumidityFanSpeedText());
            _display.writeLine(1, _makeTVOCLine());
            break;

        case _Page::CO2:
            _display.writeLine(0, _makeTemperatureHumidityFanSpeedText());
            _display.writeLine(1, _makeCO2Line());
            break;

        case _Page::PM01:
            _display.writeLine(0, _makeTemperatureHumidityFanSpeedText());
            _display.writeLine(1, _makePMLine("PM1", _deviceState.pm01));
            break;

        case _Page::PM25:
            _display.writeLine(0, _makeTemperatureHumidityFanSpeedText());
            _display.writeLine(1, _makePMLine(std::string("PM2") + _customChar(halfChar), _deviceState.pm25));
            break;

        case _Page::PM10:
            _display.writeLine(0, _makeTemperatureHumidityFanSpeedText());
            _display.writeLine(1, _makePMLine("PM10", _deviceState.pm10));
            break;

        case _Page::CalibrationState:
            _display.writeLine(0, "  Calibration   ");
            _display.writeLine(1, _makeCalibrationStateText());

        default:
            _display.writeLine(0, "     Oops!      ");
            _display.writeLine(1, " <Unknown page> ");
            break;
    }
    
    _page = page;

    _display.show();
}

void PanelDisplay::_notify(const std::string& title, const std::string& message) {
    _title = title;
    _message = message;
    _displayState = _DisplayState::StartNotification;
}

std::string PanelDisplay::_makeTemperatureHumidityFanSpeedText() {
    std::ostringstream ss;
    ss << _customChar(thermometerChar) << std::setw(4) << std::fixed << std::setprecision(1) << _deviceState.temperature << _customChar(degreesChar) << "C";
    ss << " ";
    ss << _customChar(waterDropChar) << std::setw(2) << (int)_deviceState.relHumidity << "%";
    ss << " ";
    if (_deviceState.fanSpeed < 10) ss << " ";
    ss << _customChar(airflowChar) << _deviceState.fanSpeed;
    return ss.str();
}

std::string PanelDisplay::_makeTVOCLine() {
    std::ostringstream ss;
    ss << "TVOC " << (int)_deviceState.tvoc << "ppm";
    std::string str(_fixLeft(ss.str(), _display.width() - 1));
    // Log.verboseln("### TVOC: '%s' len %d", str.c_str(), str.size());
    std::ostringstream ss2;
    _selectFaceIcon(_determineTVOCCategory(_deviceState.tvoc));
    ss2 << str << _customChar(faceChar); // TODO: reflect actual mood.
    return ss2.str();
}

std::string PanelDisplay::_makeCO2Line() {
    std::ostringstream ss;
    ss << "eCO" << _customChar(subscriptTwoChar) << " " << (int)_deviceState.co2 << "ppm";
    std::string str(_fixLeft(ss.str(), _display.width() - 1));
    std::stringstream ss2;
    _selectFaceIcon(_determineCO2Category(_deviceState.co2));
    ss2 << str << _customChar(faceChar); // TODO: reflect actual mood.
    return ss2.str();
}

std::string PanelDisplay::_makeIAQLine() {
    std::ostringstream ss;
    if (_deviceState.sensorReady) {
        ss <<  (int)_deviceState.iaq << ": " << _makeIAQDescription((int)_deviceState.iaq);
    } else {
        ss << "(measuring...)";
    }
    std::string str(ss.str());
    // TODO: Room for an emotion face on RHS!
    // return _fixLeft(str, _width);
    return _fixCentre(str, _width);
}

std::string PanelDisplay::_makePMLine(const std::string& label, const uint16_t pmLevel) {
    std::ostringstream ss;
    ss << label << " " << pmLevel << microChar << "g/m" << _customChar(superscriptThreeChar);
    std::string str(_fixLeft(ss.str(), _display.width() - 1));
    std::stringstream ss2;
    // _makeFaceAmbivalent();
    // ss2 << str << _customChar(faceChar); // TODO: reflect actual mood.
    return ss2.str();
}

std::string PanelDisplay::_makeFanSpeedNotificationMessage(int fanSpeed) {
    std::stringstream ss;
    int percent(10 * fanSpeed);
    ss << percent << "%";
    return _fixCentre(ss.str(), _width);
}

std::string PanelDisplay::_makeCalibrationStateText() {
    // TODO: impl properly.
    return " OK for 138:22  ";
}

std::string PanelDisplay::_makeIAQDescription(int iaq) {
    return _makeIAQDescription(_determineIAQCategory(iaq));
}

std::string PanelDisplay::_makeIAQDescription(_Category cat)
{
    std::string desc("?");

    switch (cat)
    {
        case _Category::Good:
            desc = "Good";
            break;

        case _Category::Moderate:
            desc = "Moderate";
            break;

        case _Category::Sensitive:
            desc = "Sensitive";
            break;

        case _Category::Unhealthy:
            desc = "Unhealthy";
            break;

        case _Category::VeryUnhealthy:
            desc = "Very Bad"; // limited in characters!
            break;

        case _Category::Hazardous:
            desc = "Hazardous";
            break;

        default:
            desc = "???";
            break;
    }

    return desc;
}

PanelDisplay::_Category PanelDisplay::_determineIAQCategory(int iaq)
{
    _Category cat(_Category::Good);

    if (iaq <=50) {
        cat = _Category::Good;
    } 
    
    else if (iaq <= 100) {
        cat = _Category::Moderate;
    } 
    
    else if (iaq <= 150) {
        cat = _Category::Sensitive;
    } 
    
    else if (iaq <= 200) {
        cat = _Category::Unhealthy;
    } 
     
    else if (iaq <= 300) {
        cat = _Category::VeryUnhealthy;
    } 
    
    else {
        cat = _Category::Hazardous;
    }

    return cat;
}

PanelDisplay::_Category PanelDisplay::_determineCO2Category(int co2)
{
    _Category cat(_Category::Good);

    if (co2 <=650) {
        cat = _Category::Good;
    } 
    
    else if (co2 <= 1500) {
        cat = _Category::Moderate;
    } 
    
    else if (co2 <= 2000) {
        cat = _Category::Sensitive;
    } 
    
    else if (co2 <= 2500) {
        cat = _Category::Unhealthy;
    } 
    
    else if (co2 <= 5000) {
        cat = _Category::VeryUnhealthy;
    } 
    
    else {
        cat = _Category::Hazardous;
    }

    return cat;
}

PanelDisplay::_Category PanelDisplay::_determineTVOCCategory(int tvoc)
{
    _Category cat(_Category::Good);

    if (tvoc <= 15) {
        cat = _Category::Good;
    } 
    
    else if (tvoc <= 25) {
        cat = _Category::Moderate;
    } 
    
    else if (tvoc <= 50) {
        cat = _Category::Sensitive;
    } 
    
    else if (tvoc <= 75) {
        cat = _Category::Unhealthy;
    } 
    
    else if (tvoc <= 100) {
        cat = _Category::VeryUnhealthy;
    } 
    
    else {
        cat = _Category::Hazardous;
    }

    return cat;
}

void PanelDisplay::_selectFaceIcon(_Category cat)  {
    _display.defineCustomChar(faceChar, const_cast<byte*>(_getFaceIconBitmap(cat)));
}

const byte* PanelDisplay::_getFaceIconBitmap(_Category cat) 
{
    const byte* bitmap;

    switch (cat)
    {
        case _Category::Good:
            bitmap = Display::bitmapVeryHappy;
            break;

        case _Category::Moderate:
            bitmap = Display::bitmapHappy;
            break;

        case _Category::Sensitive:
            bitmap = Display::bitmapAmbivalent;
            break;

        case _Category::Unhealthy:
            bitmap = Display::bitmapSad;
            break;

        case _Category::VeryUnhealthy:
            bitmap = Display::bitmapVerySad;
            break;

        case _Category::Hazardous:
            bitmap = Display::bitmapScared;
            break;

        default:
            bitmap = Display::bitmapAmbivalent;
            break;
    }

    return bitmap;
}

/*
void PanelDisplay::_makeFaceHappy() {
                                                                                                                                                                                                                                                                                                                                          _display.defineCustomChar(faceChar, const_cast<byte*>(Display::bitmapHappy));
}

void PanelDisplay::_makeFaceAmbivalent() {  
    _display.defineCustomChar(faceChar, const_cast<byte*>(Display::bitmapAmbivalent));   
}

void PanelDisplay::_makeFaceSad() {
    _display.defineCustomChar(faceChar, const_cast<byte*>(Display::bitmapSad));
}
*/

std::string PanelDisplay::_fixLeft(const std::string& str, size_t fixedLen) {
    size_t len(std::min(str.size(),  fixedLen));
    size_t padRight = fixedLen - len;
    std::string fixedStr = str.substr(0, len) + std::string(padRight, ' ');
    return fixedStr;
}

std::string PanelDisplay::_fixCentre(const std::string& str, size_t fixedLen) {
    size_t len(std::min(str.size(),  fixedLen));
    size_t pdLeft = (fixedLen - len) / 2;
    size_t padRight = fixedLen - len - pdLeft;
    std::string fixedStr = std::string(pdLeft, ' ') + str.substr(0, len) + std::string(padRight, ' ');
    return fixedStr;
}

char PanelDisplay::_customChar(char ch) {
    // Why do this? Because the underlying display will interprey
    // characters 128-135 as 0-7 and we cannot embedded char 0 into
    // any C/C++ string, so we use these alias codes.
    return 0x80 | ch;
}

