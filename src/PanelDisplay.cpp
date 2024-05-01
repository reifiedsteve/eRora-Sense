#include <sstream>
#include <iomanip>

#include "PanelDisplay.h"

PanelDisplay::PanelDisplay() 
    : _display()
    , _page(_Page::Welcome1)
    , _tempC(0)
    , _relHumidity(0)
    , _tvoc(0)
    , _co2(0)
    , _pm01(0)
    , _pm25(0)
    , _pm10(0)
    , _timer(5000)
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
    if (_timer.hasExpired()) {
        _page = _pageAfter(_page);
        _timer.restart();
    }

    _showPage(_page);
}

void PanelDisplay::onTemperature(float temperature) {
    _tempC = temperature;
}

void PanelDisplay::onHumidity(float humidity){
    _relHumidity = humidity;
}

void PanelDisplay::onTVOC(uint16_t tvoc) {
    _tvoc = tvoc;
}

void PanelDisplay::onCO2(uint16_t co2) {
    _co2 = co2;
}

void PanelDisplay::onParticleReading(uint16_t pm01, uint16_t pm25, uint16_t pm10) {
    _pm01 = pm01;
    _pm25 = pm25;
    _pm10 = pm10;
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

PanelDisplay::_Page PanelDisplay::_pageAfter(_Page page)
{
    _Page nextPage(_Page::Welcome1);

    switch (page) {
        case _Page::Welcome1:
            nextPage = _Page::Welcome2;
            break;
        case _Page::Welcome2:
            nextPage = _Page::TVOC;
            break;
        case _Page::TVOC:
            nextPage = _Page::CO2;
            break;
        case _Page::CO2:
            nextPage = _Page::PM01;
            break;
        case _Page::PM01:
            nextPage = _Page::PM25;
            break;
        case _Page::PM25:
            nextPage = _Page::PM10;
            break;
        case _Page::PM10:
            nextPage = _Page::TVOC;
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
        case _Page::Welcome1:
            _display.writeLine(0, "  eRora Sense   ");
            _display.writeLine(1, " TVOC, CO2 & PM  ");
            break;

        case _Page::Welcome2:
            _display.writeLine(0, "  eRora Sense   ");
            _display.writeLine(1, "PWM Fan Control ");
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
            _display.writeLine(1, _makePMLine("PM1", _pm01));
            break;

        case _Page::PM25:
            _display.writeLine(0, _makeTemperatureHumidityFanSpeedText());
            _display.writeLine(1, _makePMLine(std::string("PM2") + _customChar(halfChar), _pm25));
            break;

        case _Page::PM10:
            _display.writeLine(0, _makeTemperatureHumidityFanSpeedText());
            _display.writeLine(1, _makePMLine("PM10", _pm10));
            break;

        default:
            _display.writeLine(0, "     Oops!      ");
            _display.writeLine(1, " <Unknown page> ");
            break;
    }
    
    _page = page;

    _display.show();
}

std::string PanelDisplay::_makeTemperatureHumidityFanSpeedText() {
    std::ostringstream ss;
    ss << _customChar(thermometerChar) << std::setprecision(3) << std::setw(4) << _tempC << _customChar(degreesChar) << "C";
    ss << " ";
    ss << _customChar(waterDropChar) << std::setw(2) << (int)_relHumidity << "%";
    ss << " ";
    ss << _customChar(airflowChar) << "10";
    return ss.str();
}

std::string PanelDisplay::_makeTVOCLine() {
    std::ostringstream ss;
    ss << "TVOC " << _tvoc << "ppb";
    std::string str(_fixTo(ss.str(), _display.width() - 1));
    Log.verboseln("### TVOC: '%s' len %d", str.c_str(), str.size());
    std::ostringstream ss2;
    ss2 << str << _customChar(faceChar); // TODO: reflect actual mood.
    return ss2.str();
}

std::string PanelDisplay::_makeCO2Line() {
    std::ostringstream ss;
    ss << "CO" << _customChar(subscriptTwoChar) << " " << _co2 << "ppm";
    std::string str(_fixTo(ss.str(), _display.width() - 1));
    std::stringstream ss2;
    _makeFaceHappy();
    ss2 << str << _customChar(faceChar); // TODO: reflect actual mood.
    return ss2.str();
}

std::string PanelDisplay::_makePMLine(const std::string& label, const uint16_t pmLevel) {
    std::ostringstream ss;
    ss << label << " " << pmLevel << microChar << "g/m" << _customChar(superscriptThreeChar);
    std::string str(_fixTo(ss.str(), _display.width() - 1));
    std::stringstream ss2;
    _makeFaceAmbivalent();
    ss2 << str << _customChar(faceChar); // TODO: reflect actual mood.
    return ss2.str();
}

std::string PanelDisplay::_fixTo(const std::string& str, size_t fixedLen) {
    size_t len(str.size());
    std::string fixedStr = (len < fixedLen) ? (str + std::string(fixedLen - len, ' ')) : str.substr(0, fixedLen);
    Log.verboseln("#### Fixed '%s' to len %d -> '%s'", str.c_str(), fixedLen, fixedStr.c_str());
    return fixedStr;
}

void PanelDisplay::_makeFaceHappy() {
    _display.defineCustomChar(faceChar, const_cast<byte*>(Display::bitmapHappy));
}

void PanelDisplay::_makeFaceAmbivalent() {  
    _display.defineCustomChar(faceChar, const_cast<byte*>(Display::bitmapAmbivalent));   
}

void PanelDisplay::_makeFaceSad() {
    _display.defineCustomChar(faceChar, const_cast<byte*>(Display::bitmapSad));
}

char PanelDisplay::_customChar(char ch) {
    // Why do this? Because the underlying display will interprey
    // characters 128-135 as 0-7 and we cannot embedded char 0 into
    // any C/C++ string, so we use these alias codes.
    return 0x80 | ch;
}

