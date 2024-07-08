#include <SPIFFS.h> // our HTML etc. resides on SPIFFS.
#include <string>
#include <sstream>
#include <iomanip>

#include "SmartSensor.h"
#include "UserSettings.h"
#include "Diagnostics/Logging.h"

#include "Controllers/WebServerSensorController.h"

// #include "WiFiCredentials.h"
#include "MACAddress.h"
// #include "ScheduledAction.h"
#include "Utils/Splitter.h"
#include "Utils/Parser.h"
#include "Utils/Renderer.h"

// OTA takes up about 5% of flash!!!
// #define ERORA_SUPPORTS_OTA

#ifdef ERORA_SUPPORTS_OTA
#include <Update.h>
#include <AsyncElegantOTA.h>
#endif

#include <sstream>
#include <string>

#include "_Build/Version.h"

/* 

IMPORTANT NOTE:  YOU ***MUST*** READ.

Due to an issue in the underlying AsyncTCP library, whereby if both the producer and
consumer of a queue are executing on the same core, if the queue gets full, then
attempting to add to the queue results in an infinite block (and therefore triggers
the watchdog).

While not a solution per se, we can mitigate significantly by 
increasing the queue size to something that will not be exceeded for all practical
purposes. Hence we edit AsyncTCP.cpp as follows. Replace the line:

    _async_queue = xQueueCreate(32, sizeof(lwip_event_packet_t *));

with the following.

    _async_queue = xQueueCreate(128, sizeof(lwip_event_packet_t *)); // SJM 32 --> 128. Just sticking plaster though. :(

It is sticking plaster, but maybe when I have time I will delve into the AsyncTCP 
code and see if I can implement a proper fix, publishing it back to the community.

*/

// Optimisation to send silence less often than volumous audio.
#define SILENCE_OPTIMISATION

#if defined(ONBOARDING_HERE)

CaptiveRequestHandler::CaptiveRequestHandler() {}

CaptiveRequestHandler::~CaptiveRequestHandler() {}

bool CaptiveRequestHandler::canHandle(AsyncWebServerRequest *request){
    // request->addInterestingHeader("ANY");
    // return request->url() == "/wifi/ap";
    return true;
}

void CaptiveRequestHandler::handleRequest(AsyncWebServerRequest *request)
{
    Log.verboseln("CaptiveRequest: url is %s", request->url().c_str());
    Log.verboseln("CaptiveRequest: content type is \"%s\"", request->requestedConnTypeToString());
    Log.verboseln("CaptiveRequest: content len is %d", request->contentLength());

    if (request->url() == "/ws") {
        // Do nothing here at the captive portal where WS requests don't seem to work as expected.
    } 
    
    else if (request->url() == "/connecttest.txt") {
        // Some weirdness implicitly sent by some browsers. Let's just ignore it.
    } 
    
    else {
        // Any requested URL at the AP will lead to the set-up page.
        request->redirect("/wifi/install.html");
    }
}

#endif

WebServerSensorController::WebServerSensorController(
    SmartSensor& smartSensor,
    SystemSettings& systemSettings,
    UserSettings& userSettings,
    const TimeSpan& loopInterval, 
    int portNo
    // ScheduledActions& scheduledActions
) : SmartSensorController(smartSensor, "web-controller", loopInterval, Responsiveness::Always)
  , SmartSensorObserver("web-controller")
  , _systemSettings(systemSettings)
  , _userSettings(userSettings)
  , _server(portNo)
  , _ws("/ws")
  , _webSocketSendTimer(50, CountdownTimer::State::Running)
  , _webSocketSendQueue()
  , _wsCleanupTimer(500, CountdownTimer::State::Running)
  , _state()
  , _allDevices()
  , _groupUpdateInProgress(false)
  , _builder("~")
  , _wsOutputStream()
{
    // WiFi.scanNetworks(true);
}

#if 0

void WebServerSensorController::onAPSSID(const std::string& ssid) {
    _installationSSID = ssid;
}

void WebServerSensorController::onWifiSSID(const std::string& ssid) 
{
    Log.verboseln("WebServerSensorController::onWifiSSID(\"%s\")", ssid.c_str());

    std::ostringstream os;
    os << "wifi-ssid " << ssid.c_str() ;

    _webSocketSendAll(os.str());
}

void WebServerSensorController::onSignalStrength(long strength) {
    // Not currently needed by the web page.
}

void WebServerSensorController::onIPAddress(const IPAddress& ip) {

}

void WebServerSensorController::onHostName(const std::string& hostName) {

}

void WebServerSensorController::onMAC(const MACAddress& mac) {

}

void WebServerSensorController::onProgressStart(const std::string& processName) {

}

void WebServerSensorController::onProgress(uint8_t percentageComplete, const std::string& progressMessage) {

}

void WebServerSensorController::onProgressEnd() {

}

#endif

void WebServerSensorController::onSwitchOnOff(bool on) {
    _respondToSwitchOnOff(on);
}

void WebServerSensorController::onFanSpeed(int speed) {
    _respondToFanSpeed(speed);
}

void WebServerSensorController::onCabinetBrightness(uint8_t brightness) {
    _respondToCabinetBrightness(brightness);
}

void WebServerSensorController::onCabinetColour(uint8_t hue, uint8_t sat) {
    _respondToCabinetColour(hue, sat);
}

void WebServerSensorController::onTemperature(float temperature) {
    _respondToTemperature(temperature);
}

void WebServerSensorController::onHumidity(float humidity) {
    _respondToHumidity(humidity);
}

void WebServerSensorController::onAirPressure(float pressure) {
    _respondToAirPressure(pressure);
}

void WebServerSensorController::onTVOC(float tvoc) {
    _respondToTVOC(tvoc);
}

void WebServerSensorController::onCO2(float co2) {
    _respondToCO2(co2);
}

void WebServerSensorController::onIAQAvailability(bool available) {
    _respondToIAQAvailability(available);
}

void WebServerSensorController::onIAQ(float iaq) {
    _respondToIAQ(iaq);
}

void WebServerSensorController::onPM01(uint16_t pm01) {
    _respondToPM01(pm01);
}

void WebServerSensorController::onPM25(uint16_t pm25) {
    _respondToPM25(pm25);
}

void WebServerSensorController:: onPM10(uint16_t pm10) {
    _respondToPM10(pm10);
}

void WebServerSensorController::onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) {
    _respondToHeapUsage(totalHeap, freeHeap);
}

void WebServerSensorController::onFPS(unsigned fps) {
    _respondToFPS(fps);
}

void WebServerSensorController::addDevice(const DeviceInformation& device) {
    Log.verboseln("WebServerLightsController: adding device %s.", device.str().c_str());
    std::stringstream ss;
    _allDevices.push_back(device);
    _makeAddDeviceMessage(ss, device.address(), device.category(), _decodeSpaces(device.name()));
    _sendToClients(ss.str());
}

void WebServerSensorController::removeDevice(const DeviceInformation& device) {
    Log.verboseln("WebServerLightsController: removing device %s.", device.str().c_str());
    std::stringstream ss;
    _allDevices.remove_if([device](const DeviceInformation& otherDevice) {
        return otherDevice.address() == device.address(); 
    });
    _makeRemoveDeviceMessage(ss, device.address());
    _sendToClients(ss.str());
}

#if 0
void WebServerSensorController::_respondToSwitchOnOff(bool on)
{
    _state.power = on;

    if (_groupUpdateInProgress) {
        if (!_groupUpdateFirst) {
            _wsOutputStream << _messageDelimiter;
        }
        _appendPowerMessage(_wsOutputStream, on);
        _groupUpdateFirst = false;
    } 
    
    else {
        _appendPowerMessage(_wsOutputStream, on);
        _webSocketSendAll(_wsOutputStream.str().c_str());
        _wsOutputStream.clear();
    }
}
#endif

void WebServerSensorController::_respondToSwitchOnOff(bool on) {
    _state.power = on;
    std::string message(_makePowerMessage(on));
    _sendToClients(message);
}

void WebServerSensorController::_respondToFanSpeed(int speed){
    _state.fanSpeed = speed;
    std::string message(_makeFanSpeedMessage(speed));
    _sendToClients(message);
}

void WebServerSensorController::_respondToCabinetBrightness(uint8_t brightness) {

}

void WebServerSensorController::_respondToCabinetColour(uint8_t hue, uint8_t sat) {

}

void WebServerSensorController::_respondToTemperature(float temperature) {
    _state.temperature = temperature;
    std::string message(_makeTemperatureMessage(temperature));
    _sendToClients(message);
}

void WebServerSensorController::_respondToHumidity(float humidity) {
    _state.relHumidity = humidity;
    std::string message(_makeHumidityMessage(humidity));
    _sendToClients(message);
}

void WebServerSensorController::_respondToAirPressure(float pressure) {
    _state.airPressure = pressure;
    std::string message(_makeAirPressureMessage(pressure));
    _sendToClients(message);
}

void WebServerSensorController::_respondToTVOC(float tvoc) {
    _state.tvoc = tvoc;
    std::string message(_makeTVOCMessage(tvoc));
    _sendToClients(message);
}

void WebServerSensorController::_respondToCO2(float co2) {
    _state.co2 = co2;
    std::string message(_makeCO2Message(co2));
    _sendToClients(message);
}

void WebServerSensorController::_respondToIAQAvailability(bool available) {
    _state.sensorReady = available;
    std::string message(_makeIAQAvailabilityMessage(available));
    _sendToClients(message);
}

void WebServerSensorController::_respondToIAQ(float iaq) {
    _state.iaq = iaq;
    std::string message(_makeIAQMessage(iaq));
    _sendToClients(message);
}

void WebServerSensorController::_respondToPM01(uint16_t pm01) {
    _state.pm01 = pm01;
    std::string message(_makePM01Message(pm01));
    _sendToClients(message);
}

void WebServerSensorController::_respondToPM25(uint16_t pm25) {
    _state.pm25 = pm25;
    std::string message(_makePM25Message(pm25));
    _sendToClients(message);
}

void WebServerSensorController:: _respondToPM10(uint16_t pm10) {
    _state.pm10 = pm10;
    std::string message(_makePM10Message(pm10));
    _sendToClients(message);
}

void WebServerSensorController::_respondToHeapUsage(uint32_t totalHeap, uint32_t freeHeap) {
    _state.heapTotal = totalHeap;
    _state.heapFree = freeHeap;
    std::string message(_makeHeapUsageMessage(totalHeap, freeHeap));
    _sendToClients(message);
}

void WebServerSensorController::_respondToFPS(unsigned fps) {
    // _state.fps = fps;
    std::string message(_makeFPSMessage(fps));
    _sendToClients(message);
}

void WebServerSensorController::_initInputs()
{
    // **** Set up WebSockets server (only seems to work when STA, messages don't get delivered to AP endpoints).

    _ws.onEvent([this](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
        _onWebSocketEvent(server, client, type, arg, data, len);
    });

    // **** Set up the HTTP API.
   
    _server.on("/api/command", HTTP_GET, [this](AsyncWebServerRequest* request) {
        _apiHandleRequest(request);
    }).setFilter(ON_STA_FILTER);

    // **** Set up the Web UI.

    _server.on("/favicon.ico", HTTP_GET,  [this](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/favicon.ico", "image/x-icon", false, nullptr);
    });

    _server.on("/ui", HTTP_GET, [this](AsyncWebServerRequest* request) {
        _uiHandleControl(request);
    });

    // 

 #if defined(ONBOARDING_HERE)

   _server.on("/wifi", HTTP_GET, [this](AsyncWebServerRequest* request) {
        _uiHandleOnboarding(request);
    }) /* .setFilter(ON_AP_FILTER) */;

    #if false
    _server.on("/wifi", HTTP_POST, NULL, NULL, [this](AsyncWebServerRequest* request, uint8_t* bytes, size_t len, size_t index, size_t total) {
        _uiHandleOnboardingPost(request, bytes, len, index, total);
    }).setFilter(ON_AP_FILTER);
    #endif

    CaptiveRequestHandler* captiveHandler = new CaptiveRequestHandler();
    _server.addHandler(captiveHandler).setFilter(ON_AP_FILTER);

#endif

    _server.addHandler(&_ws);
    // _server.addHandler(&_ws).setFilter(ON_AP_FILTER);      // Mmm. /ws requests to CaptivePortal, but no payload.
    // _server.addHandler(&_ws).setFilter(ON_STA_FILTER);

    _server.onNotFound([this](AsyncWebServerRequest* request) {
        std::string redirectURL(ON_STA_FILTER(request) ? "/ui/index.html" : "/wifi/install.html");
        Log.errorln("WebServerLightsController: not found - \"%s\", so redirecting to \"%s\"...", request->url().c_str(), redirectURL.c_str());
        request->redirect(redirectURL.c_str());
    });

#ifdef ERORA_SUPPORTS_OTA
    AsyncElegantOTA.begin(&_server);
#endif

    _server.begin();
}

// void  WebServerSensorController::onObservationStarts(const SmartSensor& smartSensor) {
//     _smartSensor = &smartSensor;
// }

void WebServerSensorController::_serviceInputs() {
    if (_wsCleanupTimer.hasExpired()) {
        _wsCleanupTimer.restart();
        _ScopedLock lock(_wsMutex);
        _ws.cleanupClients();
    }
}

void WebServerSensorController::_makeHeapUsageMessage(std::ostream& os, uint32_t totalHeap, uint32_t freeHeap) {
    os << "heap " << totalHeap << " " << freeHeap;
}

std::string WebServerSensorController::_makeAllDevicesMessage(const _Devices& devices) {
    std::stringstream ss;
    _makeAllDevicesMessage(ss, devices);
    return ss.str();
}

void WebServerSensorController::_makeAllDevicesMessage(std::ostream& os, const _Devices& devices)
{
    _DevicesConstIter iter(devices.begin());
    _DevicesConstIter end(devices.end());

    if (iter != end) {
        _makeAddDeviceMessage(os, *iter);
        while (++iter != end) {
            os << _messageDelimiter;
            _makeAddDeviceMessage(os, *iter);
        }
    }
}

void WebServerSensorController::_makeAddDeviceMessage(std::ostream& os, const DeviceInformation& device) {
    _makeAddDeviceMessage(os, device.address(), device.category(), device.name());
}

void WebServerSensorController::_makeAddDeviceMessage(std::ostream& os, const IPAddress& address, DeviceCategory deviceType, const std::string& deviceName) {
    os << "device online " << std::string(address.toString().c_str()) << " " << _toString((int)deviceType) << " " << _encodeSpaces(deviceName);
}

void WebServerSensorController::_makeRemoveDeviceMessage(std::ostream& os, const IPAddress& address) {
    os << "device offline " << std::string(address.toString().c_str());
}

void WebServerSensorController::_onWebSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    switch(type)
    {
        case WS_EVT_CONNECT:
            Log.verboseln("WebServerSensorController: _onWebSocketEvent (CONNECT), from %s", client->remoteIP().toString().c_str());
            _onWebSocketConnect(server, client, arg, data, len);
            break;

        case WS_EVT_DISCONNECT:
            Log.verboseln("WebServerSensorController: _onWebSocketEvent (DISCONNECT), from %s", client->remoteIP().toString().c_str());
            _onWebSocketDisconnect(server, client, arg, data, len);
            break;

        case WS_EVT_ERROR:
            Log.verboseln("WebServerSensorController: _onWebSocketEvent (ERROR), from %s", client->remoteIP().toString().c_str());
            _onWebSocketError(server, client, arg, data, len);
            break;

        case WS_EVT_PONG:
            Log.verboseln("WebServerSensorController: _onWebSocketEvent (PONG), from %s", client->remoteIP().toString().c_str());
            _onWebSocketPong(server, client, arg, data, len);
            break;

        case WS_EVT_DATA:
            Log.verboseln("WebServerSensorController: _onWebSocketEvent (DATA), from %s", client->remoteIP().toString().c_str());
            _onWebSocketMessage(server, client, arg, data, len);
            break;

        default:
            Log.warningln("WebServerSensorController: unknown web socket event type of %d from %s.", type, client->remoteIP().toString().c_str());
            break;
    }
}

static int totalNoOfConnectedClients = 0;

void WebServerSensorController::_onWebSocketConnect(AsyncWebSocket * server, AsyncWebSocketClient * client, void * arg, uint8_t *data, size_t len)
{
    ++totalNoOfConnectedClients;
    Log.infoln("WebServerSensorController: web client connected (total now connected is %d).", totalNoOfConnectedClients);
    
    // if (_smartSensor)
    // {
        Log.verboseln("WebServerSensorController: sending current sensors state to web clients...");

        // Send all information in one single ws message in order to 
        // put less stress on the fragile underlying ws implementation.

        std::stringstream ss;

        _appendAllUserSettingsMessage(ss);
        ss << _messageDelimiter;

        _appendAllControlStates(ss);
        ss << _messageDelimiter;

        _appendAllSensorReadings(ss);
        ss << _messageDelimiter;
        
        std::string devicesMessage(_makeAllDevicesMessage(_allDevices));

        if (devicesMessage.size() > 0) {
            ss << _messageDelimiter;
            ss << devicesMessage;
        }

        std::string message(ss.str());

        if (message.size() > 0) {
            Log.verboseln("WebServerSensorController: sending message \"%s\".", message.c_str());
            client->text(message.c_str());
        }
    // }

    // else {
    //    Log.warningln("WebServerSensorController: no smart sensor, so cannot get user settings to send!");
    // }
}

void WebServerSensorController::_onWebSocketDisconnect(AsyncWebSocket * server, AsyncWebSocketClient * client, void * arg, uint8_t *data, size_t len) {
    --totalNoOfConnectedClients;
    Log.infoln("WebServerSensorController: web client disconnected (total now connected is %d).", totalNoOfConnectedClients);
    // Nothing to do (yet?).
    // TODO: note: We could keep track of each client connecting/disconnecting and do textAll ourselves...?
    // This then gives us access to client.isQueueFull() and if so avoid sending at that time.
}

void WebServerSensorController::_onWebSocketError(AsyncWebSocket * server, AsyncWebSocketClient * client, void * arg, uint8_t *data, size_t len) {
    // Nothing to do (yet?).
}

void WebServerSensorController::_onWebSocketPong(AsyncWebSocket * server, AsyncWebSocketClient * client, void * arg, uint8_t *data, size_t len) {
    // Nothing to do (yet?).
}

void WebServerSensorController::_onWebSocketMessage(AsyncWebSocket * server, AsyncWebSocketClient * client, void * arg, uint8_t *data, size_t len)
{
    AwsFrameInfo* info = (AwsFrameInfo*)arg;

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        std::string message((char*)data, len);
        _parseRequest(message);
    }
}

void WebServerSensorController::_apiHandleRequest(AsyncWebServerRequest* request) 
{
    bool handled(false);

    size_t n(request->params());

    if (n > 0) 
    {
        handled = true;

        bool query(false);

        if (n==1)
        {
            AsyncWebParameter* param(request->getParam(0));
            std::string command(_normalised(std::string(param->name().c_str())));

            if (command == "query") {
                std::string itemToQuery(_normalised(std::string(param->value().c_str())));
                handled = _apiHandleStateQuery(request, itemToQuery);
                query = true;
            }            
        }

        if (!query) {
            handled = _apiHandleCommands(request);
        }
    }

    if (!handled) {
        Log.warningln("Unhandled web request.");
    }
}

bool WebServerSensorController::_apiHandleStateQuery(AsyncWebServerRequest* request, const std::string& queryItemName)
{
    bool handled(true);

    if (queryItemName == "power") {
        bool on(_state.power);
        request->send(200, "text/plain", (on ? "on" : "off"));
    }

    else if (queryItemName == "fan-speed") {
        int fanSpeed(_state.fanSpeed); 
        request->send(200, "text/plain", String(_toString(fanSpeed).c_str()));
    }

    else if (queryItemName == "iaq") {
        int iaq((int)_state.iaq); 
        request->send(200, "text/plain", String(_toString(iaq).c_str()));
    }

    else if (queryItemName == "tvoc") {
        int tvoc((int)_state.tvoc); 
        request->send(200, "text/plain", String(_toString(tvoc).c_str()));
    }

    else if (queryItemName == "co2") {
        int co2((int)_state.co2); 
        request->send(200, "text/plain", String(_toString(co2).c_str()));
    }

    else if (queryItemName == "temperature") {
        float tempC(_state.temperature); 
        request->send(200, "text/plain", String(_toString(tempC, 1).c_str()));
    }

    else if (queryItemName == "humidity") {
        float relHumidity(_state.relHumidity); 
        request->send(200, "text/plain", String(_toString(relHumidity, 1).c_str()));
    }

    else if ((queryItemName == "air-pressure") || (queryItemName == "pressure")) {
        int pressure((int)_state.airPressure);
        request->send(200, "text/plain", String(_toString(pressure).c_str()));
    }

    else {
        request->send(404, "text/plain", "Invalid API query.");
        Log.errorln("WebServerSensorController::_apiHandleRequest: unhandled HTTP API query.");
        handled = false;
    }

    return handled;
}

bool WebServerSensorController::_apiHandleCommands(AsyncWebServerRequest* request)
{
    bool handled(true);

    size_t n(request->params());
    size_t i(0);

    while (handled && (i < n)) {
        AsyncWebParameter* param(request->getParam(i++));
        std::string command(_normalised(std::string(param->name().c_str())));
        std::string value(_normalised(std::string(param->value().c_str())));
        handled = handled && _apiHandleCommand(command, value);
    }

    if (handled) {
        request->send(200, "text/plain", "OK");
    } 
    
    else {
        request->send(404, "text/plain", "Invalid API command.");
        Log.errorln("WebServerSensorController::_apiHandleRequest: unhandled HTTP API request.");
    }

    return handled;
}

bool WebServerSensorController::_apiHandleCommand(const std::string& command, const std::string& value) 
{
    bool handled(false);

    if (command == "power") {
        handled = _parsePowerRequest(value);
    }

    else if ((command == "fan-speed") || (command == "level")) {
        handled = _parseFanSpeedRequest(value);
    }

    /*
    else if ((command == "color") || (command == "colour") || (command == "rgb")) {
        handled = _parseLightRGBRequest(value);
    }
    */

    return handled;
}

void WebServerSensorController::_sendUserSettings(AsyncWebSocketClient* client)
{
    std::stringstream ss;
    _appendAllUserSettingsMessage(ss);

    std::string message(ss.str());

    if (message.size() > 0) {
        // Log.verboseln("WebServerSensorController: sending message \"%s\".", message.c_str());
        client->text(message.c_str());
    }

    Log.verboseln("WebServerSensorController: sent settings message.");
}

void WebServerSensorController::_appendAllControlStates(std::ostream& os)
{
    _appendPowerMessage(os, _state.power);
    os << _messageDelimiter;

    _appendFanSpeedMessage(os, _state.fanSpeed);     
}

void WebServerSensorController::_appendAllSensorReadings(std::ostream& os)
{
    _appendTemperatureMessage(os, _state.temperature);
    os << _messageDelimiter;

    _appendHumidityMessage(os, _state.relHumidity);
    os << _messageDelimiter;
        
    _appendAirPressureMessage(os, _state.airPressure);
    os << _messageDelimiter;

    _appendIAQMessage(os, _state.iaq); ;
    os << _messageDelimiter;

    _appendTVOCMessage(os, _state.tvoc);
    os << _messageDelimiter;

    _appendCO2Message(os, _state.co2);
    os << _messageDelimiter;

    _appendIAQAvailabilityMessage(os, _state.sensorReady);
    os << _messageDelimiter;

    #if 0

    os << _messageDelimiter;
    os << "pm1 " << (int)_state.pm01;

    os << _messageDelimiter;
    os << "pm25 " << (int)_state.pm25;

    os << _messageDelimiter;
    os << "pm10 " << (int)_state.pm10;

    #endif
}

void WebServerSensorController::_appendAllUserSettingsMessage(std::ostream& os)
{
    // Log.verboseln("WebServerSensorController: sending settings message...");

    # if 0

    _appendUserSettingMessage(os, "transition-type", (int)_userSettings.getEffectTransitionType());
    os << _messageDelimiter;
    
    _appendUserSettingMessage(os, "transition-time", (int)_userSettings.getEffectTransitionTime().get(TimeSpan::Units::Milliseconds));
    os << _messageDelimiter;
    
    #endif
}

std::string WebServerSensorController::_makePowerMessage(bool on) {
    std::ostringstream os;
    _appendPowerMessage(os, on);
    return os.str();
}

std::string WebServerSensorController::_makeFanSpeedMessage(int speed) {
    std::ostringstream os;
    _appendFanSpeedMessage(os, speed);
    return os.str();
}

std::string WebServerSensorController::_makeTemperatureMessage(float temperature) {
    std::ostringstream os;
    _appendTemperatureMessage(os, temperature);
    return os.str();
}

std::string WebServerSensorController::_makeHumidityMessage(float humidity) {
    std::ostringstream os;
    _appendHumidityMessage(os, humidity);
    return os.str();
}

std::string WebServerSensorController::_makeAirPressureMessage(float pressure) {
    std::ostringstream os;
    _appendAirPressureMessage(os, pressure);
    return os.str();
}

std::string WebServerSensorController::_makeTVOCMessage(float tvoc) {
    std::ostringstream os;
    _appendTVOCMessage(os, tvoc);
    return os.str();
}

std::string WebServerSensorController::_makeCO2Message(float co2) {
    std::ostringstream os;
    _appendCO2Message(os, co2);
    return os.str();
}

std::string WebServerSensorController::_makeIAQAvailabilityMessage(bool available) {
    std::ostringstream os;
    _appendIAQAvailabilityMessage(os, available);
    return os.str();
}

std::string WebServerSensorController::_makeIAQMessage(float iaq) {
    std::ostringstream os;
    _appendIAQMessage(os, iaq);
    return os.str();
}

std::string WebServerSensorController::_makePM01Message(uint16_t pm01) {
    std::ostringstream os;
    _appendPM01Message(os, pm01);
    return os.str();
}

std::string WebServerSensorController::_makePM25Message(uint16_t pm25) {
    std::ostringstream os;
    _appendPM25Message(os, pm25);
    return os.str();
}

std::string WebServerSensorController::_makePM10Message(uint16_t pm10) {
    std::ostringstream os;
    _appendPM10Message(os, pm10);
    return os.str();
}

std::string WebServerSensorController::_makeHeapUsageMessage(uint32_t totalHeap, uint32_t freeHeap) {
    std::ostringstream os;
    _appendHeapUsageMessage(os, totalHeap, freeHeap);
    return os.str();
}
    
std::string WebServerSensorController::_makeFPSMessage(unsigned fps) {
    std::ostringstream os;
    _appendFPSMessage(os, fps);
    return os.str();
}
    
void WebServerSensorController::_appendPowerMessage(std::ostream& os, bool on) {
    os << "power " << _toBoolString(on);
}

void WebServerSensorController::_appendFanSpeedMessage(std::ostream& os, int speed) {
    os << "fan-speed " << speed;
}

void WebServerSensorController::_appendTemperatureMessage(std::ostream& os, float temperature) {
    os << "temperature " << _toString(temperature, 1);
}

void WebServerSensorController::_appendHumidityMessage(std::ostream& os, float humidity) {
    os << "humidity " << _toString(humidity, 1);
}

void WebServerSensorController::_appendAirPressureMessage(std::ostream& os, float pressure) {
    os << "air-pressure " << (int)pressure;
}

void WebServerSensorController::_appendTVOCMessage(std::ostream& os, float tvoc) {
    os << "tvoc " << (int)tvoc;
}

void WebServerSensorController::_appendCO2Message(std::ostream& os, float co2) {
    os << "co2 " << (int)co2;
}

void WebServerSensorController::_appendIAQAvailabilityMessage(std::ostream& os, bool available) {
    os << "iaq-ready " << _toBoolString(available);
}

void WebServerSensorController::_appendIAQMessage(std::ostream& os, float iaq) {
    os << "iaq " << (int)iaq;
}

void WebServerSensorController::_appendPM01Message(std::ostream& os, uint16_t pm01) {
    os << "pm1 " << pm01;
}

void WebServerSensorController::_appendPM25Message(std::ostream& os, uint16_t pm25) {
    os << "pm25 " << pm25;
}

void WebServerSensorController:: _appendPM10Message(std::ostream& os, uint16_t pm10) {
    os << "pm10 " << pm10;
}

void WebServerSensorController::_appendHeapUsageMessage(std::ostream& os, uint32_t totalHeap, uint32_t freeHeap) {
    os << "heap " << totalHeap << " " << freeHeap;
}

void WebServerSensorController::_appendFPSMessage(std::ostream& os, unsigned fps) {
    os << "fps " << fps;
}

void WebServerSensorController::_appendUserSettingMessage(std::ostream& os, const char* settingName, int value) {
    os << "setting " << settingName << " " << value;
}

void WebServerSensorController::_appendUserSettingMessage(std::ostream& os, const char* settingName, unsigned value) {
    os << "setting " << settingName << " " << value;
}

void WebServerSensorController::_appendUserSettingMessage(std::ostream& os, const char* settingName, bool state) {
    _appendUserSettingMessage(os, settingName, (state ? "on" : "off"));
}

void WebServerSensorController::_appendUserSettingMessage(std::ostream& os, const char* settingName, const std::string& str) {
    _appendUserSettingMessage(os, settingName, str.c_str());
}

void WebServerSensorController::_appendUserSettingMessage(std::ostream& os, const char* settingName, const char* str) {
    os << "setting " << settingName << " " << str;
}

#if 0

void WebServerLightsController::_appendScheduledActionsMessage(std::ostream& os, const ScheduledActions& scheduledActions, char delimiter)
{
    int n(_scheduledActions.size());

    for (int timerNo = 1; timerNo <= n; timerNo++) {
        ScheduledAction scheduledAction(_scheduledActions.get(timerNo));
        if (timerNo > 1) os << delimiter;
        _appendScheduledActionMessage(os, timerNo, scheduledAction);
    }
}

void WebServerLightsController::_appendScheduledActionMessage(std::ostream& os, int timerNo, const ScheduledAction& scheduledAction)
{
    os << "timer " << timerNo << " " << scheduledAction.str();
}

std::string WebServerLightsController::_makeScheduledActionMessage(int timerNo, const ScheduledAction& scheduledAction) {
    std::stringstream ss;
    _appendScheduledActionMessage(ss, timerNo, scheduledAction);
    return ss.str();
}

#endif

void WebServerSensorController::_parseRequest(const std::string& message)
{
    // There may be multiple commands present in a single message. If so, they
    // are delimited with a tilda('~') and we need to split them apart and 
    // process each distinctly.
    // Being able to aggregate multiple messages in this way is useful as the
    // underlying async socket class seems fragile when given too many messages
    // in quick succession.

    std::vector<std::string> messages;
    int n(Splitter::split(message, std::inserter(messages, messages.begin()), '~'));

    Log.verboseln("WebServerSensorController: received %d message(s) in a single ws delivery.", n);

    for (int i = 0; i < n; ++i)
    {
        const std::string& singleMessage(messages[i]);

        Log.verboseln("WebServerSensorController: rx message \"%s\"", singleMessage.c_str());
        bool handled(_parseSingleRequest(singleMessage));

        if (!handled) {
            Log.errorln("WebServerSensorController: unhandled websocket request \"%s\".", singleMessage.c_str());
        }
    }
}

bool WebServerSensorController::_parseSingleRequest(const std::string& message) 
{
    bool handled(false);

    std::vector<std::string> parts;

    if (message == "reboot") {
        // Bit harsh to do this without warning, maybe?
        _reboot();
    }

    else if (_split(message, ' ', parts))
    {
        if (parts.size() > 1)
        {
            std::string command(_normalised(parts[0]));

            if (command == "power") {
                const std::string& stateStr(parts[1].c_str());
                handled = _parsePowerRequest(stateStr);
            }

            else if ((command == "fan-speed") || (command == "level")) {
                const std::string& speedStr(parts[1].c_str());
                handled = _parseFanSpeedRequest(speedStr);
            }

        }
    }

    return handled;
}

bool WebServerSensorController::_parsePowerRequest(const std::string& value)
{
    bool handled(true);

    if (value == "on") {
        _switchOnOff(true);
    }
    
    else if (value == "off") {
        _switchOnOff(false);    
    }
    
    else if (value == "toggle") {
        _toggleOnOff();
    }

    else {
        handled = false;
    }

    return handled;
}

bool WebServerSensorController::_parseFanSpeedRequest(const std::string& value)
{
    bool handled(false);

    // Note HTTP params strip off any preceding '+', so we allow for '>' too.

    bool isNeg(false);
    bool isAdjustment(false);

    int valueStart(0);

    if (_startsWith(value, "-") || _startsWith(value, "<")) {
        valueStart = 1;
        isAdjustment = true;
        isNeg = true;
    }

    else if (_startsWith(value, "+") || _startsWith(value, ">")) {
        valueStart = 1;
        isAdjustment = true;
        isNeg = false;
    }

    int amount;
    bool parsed(Parser::tryParse(value.substr(valueStart), amount));

    if (parsed)
    {
        amount = amount * (isNeg ? -1 : 1);
        
        if (isAdjustment)
        {
            if ((std::numeric_limits<int8_t>::min() <= amount) && (amount <= std::numeric_limits<int8_t>::max())) {
                _adjustFanSpeed((int8_t)amount);
                handled = true;
            }
        }
        
        else
        {
            if ((std::numeric_limits<uint8_t>::min() <= amount) && (amount <= std::numeric_limits<uint8_t>::max())) {
                _setFanSpeed((uint8_t)amount);
                handled = true;
            }
        }
    }

    return handled;
}

bool WebServerSensorController::_parseUserSettingRequest(const std::string& settingName, const std::string& valueStr)
{
    bool handled(true);

    #if 0

    if (settingName == "show-preview")
    {
        bool on(false);

        if (valueStr == "toggle") {
            _togglePreview();
        }
        
        else {
            handled = Parser::tryParse(valueStr, on);
            if (handled) {
                _enablePreview(on);
            }
        }
    }

    else if (settingName == "transition-type") {
        int type;
        handled = Parser::tryParse(valueStr, type);
        if (handled) {
            _setTransitionType((TransitionType)type);
        }
    }

    else if (settingName == "transition-time") {
        int periodMs;
        handled = Parser::tryParse(valueStr, periodMs);
        if (handled) {
            TimeSpan period(periodMs, TimeSpan::Units::Milliseconds);
            _setTransitionTime(period);
        }
    }

    else {
        handled = false;
        Log.warningln("WebserverLightsController: unknown user setting of \"%s\".", settingName.c_str());
    }

    #endif

    return handled;
}

#if 0

bool WebServerSensorController::_parseTimer(const std::string& timerNoStr, const std::string& timeStr, const std::string& daysStr, const std::string& actionStr)
{
    bool handled(false);

    int timerNo;

    if (Parser::tryParse(timerNoStr, timerNo))
    {
        ScheduledAction scheduledAction;

        if (ScheduledAction::tryParse(timeStr, daysStr, actionStr, scheduledAction)) {
            Log.verbose("WebServerSensorController: parsed \"%s\" to \"%s\".", actionStr.c_str(), scheduledAction.str().c_str());
            _scheduledActions.set(timerNo, scheduledAction);
            _webSocketSendAll(_makeScheduledActionMessage(timerNo, scheduledAction));
            handled = true;
        }

        else {
            Log.errorln("WebServerSensorController: error parsing scheduled action \"%s %s %s\".", 
                timeStr.c_str(), daysStr.c_str(), actionStr.c_str()
            );
        }
    }

    else {
        Log.errorln("WebServerSensorController: error parsing timer no \"%s\".", timerNoStr.c_str());
    }

    return handled;
}

#endif

#if defined(ONBOARDING_HERE)

void WebServerSensorController::_uiHandleOnboarding(AsyncWebServerRequest *request) 
{
    // We use a form submission here as for some reason websockets appear to be problematic
    // with an AP. e.g. WS packets arrive but without a payload.

    Log.verboseln("WebServerSensorController:_uiHandleOnboarding: url is \"%s\", via %s", 
        request->url().c_str(),
        ON_AP_FILTER(request) ? "AP" : "STA"
    );

    std::string errorStr("");

    String url(request->url());
    
    if (url.endsWith("/wifi/restarting.html"))
    {
        Log.verboseln("WebServerSensorController::_uiHandleOnboarding ends with \"restarting.html\"");

        LabelledValues parameters(_getParametersFromMethodGET(request));

        int n(parameters.size());

        for (int i = 0; i < n; ++i) {
            const std::string& label(parameters.label(i));
            const std::string& value(parameters.value(i));
            Log.verboseln("WebServerSensorController: parameter \"%s\" = \"%s\".", label.c_str(), value.c_str());
        }

        _applySettings(parameters);

        _switchOnOff(false);
        _serveFromFile(request);

        sleep(5);
        _reboot();

    }

    else if (url.endsWith("rebooting.html")) {
            _switchOnOff(false);
            _serveFromFile(request);
            sleep(5);
            _reboot();
    }

    else {
        Log.verboseln("WebServerSensorController::_uiHandleOnboarding serving from file.");
        _serveFromFile(request);
        vTaskDelay(1); // Feed the dog as there could be a fair few files.
    } 
}

#endif

std::string WebServerSensorController::_applySettings(const LabelledValues& settings)
{
    std::string errorStr("");

    // Prevent settings being committed to flash after each and
    // every setting is set.

    _systemSettings.setCommitMode(Settings::CommitMode::Manual);

    size_t n(settings.size());

    for (int i = 0; i < n; ++i) 
    {
        const std::string& name(settings.label(i));
        const std::string& value(settings.value(i));

        if ((name == "NA") || (name == "network-auto")) {
            bool netAuto(true);
            if (Parser::tryParse(value, netAuto)) {
                _systemSettings.useManualNetworkConfig(!netAuto);
            } else {
                errorStr = "Error: invalid net config option \"" + value + "\".";
            }
        }

        else if ((name == "SI") || (name == "static-ip")) {
            IPAddress ip;
            if (ip.fromString(String(value.c_str()))) {
                _systemSettings.setStaticIP(ip);
            }
        }

        else if ((name == "GA") || (name == "gateway")) {
            IPAddress ip;
            if (ip.fromString(String(value.c_str()))) {
                _systemSettings.setGateway(ip);
            }
        }

        else if ((name == "SN") || (name == "subnet")) {
            IPAddress ip;
            if (ip.fromString(String(value.c_str()))) {
                _systemSettings.setSubnet(ip);
            }
        }

        else if ((name == "D1") || (name == "dns-1")) {
            IPAddress ip;
            if (ip.fromString(String(value.c_str()))) {
                _systemSettings.setDNS1(ip);
            }
        }

        else if ((name == "D2") || (name == "dns-2")) {
            IPAddress ip;
            if (ip.fromString(String(value.c_str()))) {
                _systemSettings.setDNS2(ip);
            }
        }

        else if ((name == "WS") || (name == "wifi-ssid")) {
            _systemSettings.setWifiSSID(value);
        }
    
        else if ((name == "WP") || (name == "wifi-psk")) {
            _systemSettings.setWifiPSK(value);
        }

        else if ((name == "DN") || (name == "device-name")) {
            _systemSettings.setDeviceDescriptiveName(value);
        }

        else if ((name == "LC") || (name == "led-count"))
        {
            int numLEDs(0);

            if (!Parser::tryParse(value, numLEDs)) {
                errorStr = "Missing: number of LEDs.";
            } 
            
            else if (numLEDs <= 0) {
                errorStr = "Error: Number of LEDs is incorrect.";
            }

            else {
                _systemSettings.setNumberOfLEDs(numLEDs);
            }
        }

        /*

        else if ((name == "GC") || (name == "gamma-correct"))
        {
            bool on(false);
            if (Parser::tryParse(value, on)) {
                _systemSettings.setGammaCorrect(on);
            } else {
                errorStr = "Error: invalid gamma correction option \"" + value + "\".";
            }
        }

        */

        else if ((name == "PC") || (name == "psu-milliamps"))
        {
            int psuMilliamps(0);

            if (!Parser::tryParse(value, psuMilliamps)) {
                errorStr = "Missing: PSU milliamps.";
            } 
            
            else if (psuMilliamps <= 0) {
                errorStr = "Error: PSU milliamps is incorrect.";
            }

            else {
                _systemSettings.setLEDsPSUMilliamps(psuMilliamps);
            }
        }

        else if ((name == "PM") || (name == "manage-power")) {
            bool managePower(false);
            if (Parser::tryParse(value, managePower)) {
                _systemSettings.setManagePower(managePower);
            }
        }

        else if ((name == "QA") || (name == "mqtt-address")) {
            // Could be ip or domain name.
            _systemSettings.setMQTTBrokerAddress(value);
        }

        else if ((name == "QC") || (name == "mqtt-port"))
        {
            int port(0);

            if (!Parser::tryParse(value, port)) {
                errorStr = "Missing: MQTT port.";
            } 
            
            else if (port <= 0) {
                errorStr = "Error: MQTT port is incorrect.";
            }

            else {
                _systemSettings.setMQTTBrokerPort(port);
            }
        }

        else if ((name == "QT") || (name == "mqtt-topic")) {
            _systemSettings.setMQTTTopicPrefix(value);
        }

        else if ((name == "QU") || (name == "mqtt-username")) {
            _systemSettings.setMQTTBrokerUsername(value);
        }

        else if ((name == "QP") || (name == "mqtt-password")) {
            _systemSettings.setMQTTBrokerPassword(value);
        }

        #if 0

        else if ((name == "NT") || (name == "ntp-domain")) {
            _systemSettings.setNTPDomain(value);
        }

        else if ((name == "TZ") || (name == "timezone")) {
            _systemSettings.setTimezone(value);
        }

        else if ((name == "TN") || (name == "timezone-name")) {
            _systemSettings.setTimezoneName(value);
        }

        #endif

        else {
            Log.errorln("WebServerSensorController: unknown setting named \"%s\".", name.c_str());
        }
    }

    _systemSettings.commit();
    _systemSettings.setCommitMode(Settings::CommitMode::Auto);

    return errorStr;
}

LabelledValues WebServerSensorController::_getParametersFromMethodGET(AsyncWebServerRequest *request) 
{
    LabelledValues parameters;

    if (request->hasParam("settings"))
    {
        std::vector<std::string> settings;

        std::string settingsStr(request->getParam("settings")->value().c_str());
        Splitter::split(settingsStr, std::back_inserter(settings), ',');

        for (const std::string& setting : settings) 
        {
            std::vector<std::string> parts;
            Splitter::split(setting, std::back_inserter(parts), ':', true, 2);

            if (parts.size() == 2) {
                const std::string& name(parts[0]);
                const std::string& value(parts[1]);
                parameters.add(name, value);
            }
        }
    }

    else {
        size_t n(request->params());

        for (size_t i = 0; i < n; ++i) {
            AsyncWebParameter* param = request->getParam(i);
            parameters.add(param->name().c_str(), param->value().c_str());
        }
    }

    return parameters;
}

void WebServerSensorController::_uiHandleControl(AsyncWebServerRequest *request) 
{
    Log.verboseln("WebServerLightsController:_uiHandleControl: url is \"%s\", via %s", 
        request->url().c_str(),
        ON_AP_FILTER(request) ? "AP" : "STA"
    );

    _serveFromFile(request);
}

void WebServerSensorController::_serveFromFile(AsyncWebServerRequest *request)
{
    typedef std::function<String(const String&)> TemplateProcessor;

    TemplateProcessor templateProcessor(
        [this](const String& var) { return _processTemplates(var); }
    );

    // Generalise -> extract path &  filename from request. Assumed mirrored 
    // on FS. Determine type from extension. 

    String url(request->url());

    Log.verboseln("WebServerSensorController: serving from file, url \"%s\"...", url.c_str());

    if (url.endsWith("/")) {
        url = url + "index.html";
    }

    if (url.endsWith(".html")) {
        request->send(SPIFFS, url, "text/html", false, templateProcessor);
    } 
    
    else if (url.endsWith(".css")) {
        request->send(SPIFFS, url, "text/css", false, nullptr);
    }

    else if (url.endsWith(".js")) {
        request->send(SPIFFS, url, "application/javascript", false, nullptr);
    }

    else if (url.endsWith(".png")) {
        request->send(SPIFFS, url, "image/png", false, nullptr);
    }

    else if (url.endsWith(".svg")) {
        request->send(SPIFFS, url, "image/svg+xml", false, nullptr);
    }

    else if (url.endsWith(".ico")) {
        request->send(SPIFFS, url, "image/png", false, nullptr);
    }

    else {
        // Default to main page.
        //Log.errorln("Webserver did not server up %s", url.c_str());
        //request->send(SPIFFS, "/signin.html", "text/html", false, templateProcessor);
    }

    // Log.verboseln("WebServerLightsController: served url \"%s\"...", url.c_str());
}

String WebServerSensorController::_processTemplates(const String& key) 
{
    Log.verboseln("Template substituting value for key of \"%%%s%%\"", key.c_str());
    Log.verboseln("WebServerLightsController: template processor given key \"%s\"", key.c_str());

    std::string value("");

    if (key == "VERSION") {
        value = BUILD_VERSION;
    }

    else if (key == "BUILDDATE") {
        value = BUILD_DATE;
    }

    else if (key == "BUILDTIME") {
        value = BUILD_TIME;
    }

    else if (key == "SSID") {
        value = _systemSettings.getWifiSSID();
    }

    else if (key == "PSK") {
        value = _systemSettings.getWifiPSK();
    }

    else if (key == "APSSID") {
        value = _installationSSID;
    }

    else if (key == "APPSK") {  
        value = "TBD"; // ??? Generate a 4-digit pin and show on OLED ??? Or optional?
    }

    else if (key == "MACADDR") {
        value = MACAddress::local().str(":");
    }

    else if (key == "IPADDR") {
        value = std::string(WiFi.localIP().toString().c_str());
    }

    else if (key == "LEDCOUNT") {
        value = _toString(_systemSettings.getNumberOfLEDs());
    }

    #if 0

    else if (key == "LEDDENSITY") {
        value = _toString(_userSettings.getNumberOfLEDsPerMetre());
    }

    else if (key == "GAMMACORRECT") {
        value = _systemSettings.getGammaCorrect() ? "yes" : "no";
    }

    #endif

    /***
     
    We currently only support WS2812B, hence voltage is fixed at 5v.

    else if (key == "PSUVOLTS") {
        value = String(_userSettings.getLEDsPSUVoltage());
    }
    ***/

    else if (key == "PSUMILLIAMPS") {
        value = _toString(_systemSettings.getLEDsPSUMilliamps());
    }

    else if (key == "PSUMANAGE") {
        value = _systemSettings.getManagePower() ? "on" : "off";
    }

    else if (key == "MQTTADDR") {
        value = _systemSettings.getMQTTBrokerAddress();
    }

    else if (key == "MQTTPORT") {
        value = _toString(_systemSettings.getMQTTBrokerPort());
    }

    else if (key == "MQTTTOPICPREFIX") {
        value = _systemSettings.getMQTTTopicPrefix();
    }

    else if (key == "MQTTUSERNAME") {
        value = _systemSettings.getMQTTBrokerUsername();
    }

    else if (key == "MQTTPASSWORD") {
        value = _systemSettings.getMQTTBrokerPassword();
    }

    #if 0

    else if (key == "NTPDOMAIN") {
        value = _systemSettings.getNTPDomain();
    }

    else if (key == "LANITEMS") 
    {
        // int n = WiFi.scanNetworks(); /// TODO: Does this take too long and cause async_udp task to trigger watchdog????? YES, it does!!!
        // So we use the async variant instead. Though page-refreshing will be required.

        int n = WiFi.scanComplete();

        if (n < 0) {
            Log.verboseln("WebServerSensorController: beginning wifi scan...");
            WiFi.scanNetworks(true); // true means async.
        }

        else if (n > 0)
        {
            Log.verboseln("WebServerSensorController: finished wifi scan and found %d networks.", n);

            std::ostringstream os;

            for (int i = 0; i < n; ++i)
            {
                String ssid(WiFi.SSID(i));

                if (ssid.length() > 0) { // Assumption: hidden APs give no name?
                    // TODO: the HTML needs to be client-side, not here.
                    os << "<tr>" << std::endl;
                    os << "  <td class=\"lanSig\">" << _rssiToQuality(WiFi.RSSI(i)) << "</td>" << std::endl;
                    os << "  <td class=\"lanSec\">" << ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? '-' : '*') << "</td>" << std::endl;
                    os << "  <td class=\"lanName\" onclick=\"onClickLAN(this)\">" << ssid.c_str() << "</td>" << std::endl;
                    os << "</tr>" << std::endl;
                }
            }

            value = os.str();

            WiFi.scanDelete();

            if(WiFi.scanComplete() < 0) {
                Log.verboseln("WebServerSensorController: restarting wifi scan...");
                WiFi.scanNetworks(true);
            }
        }
    }

    #endif

    else if (key == "DEVICENAME") {
        value = _systemSettings.getDeviceDescriptiveName();
    }

    else if (key == "DEVICETYPE") {
        value = "LEDs";
    }

    else if (key == "NETAUTO") {
        value = _toBoolString(!_systemSettings.useManualNetworkConfig());
    }

    else if (key == "NETSTATICIP") {
        value = std::string(_systemSettings.getStaticIP().toString().c_str());
    }

    else if (key == "NETSUBNET") {
        value = std::string(_systemSettings.getSubnet().toString().c_str());
    }

    else if (key == "NETGATEWAY") {
        value = std::string(_systemSettings.getGateway().toString().c_str());
    }

    else if (key == "NETDNS1") {
        value = std::string(_systemSettings.getDNS1().toString().c_str());
    }

    else if (key == "NETDNS2") {
        value = std::string(_systemSettings.getDNS2().toString().c_str());
    }

    #if 0

    else if (key == "TIMEZONENAME") {
        value = std::string(_systemSettings.getTimezoneName().c_str());
    }

    else if (key == "TIMEZONESPEC") {
        value = std::string(_systemSettings.getTimezone().c_str());
    }

    #endif

    /*
    else if (key == "INSTALLPAGEERRORMESSAGE") {
        value = _installPageErrorMessage;
    }
    */

    // Log.verboseln("WebServerSensorController: template processor says \"%s\" = \"%s\"", key.c_str(), value.c_str());

    // Log.verboseln("WiFiInitiator: template processor returns value \"%s\"", value.c_str());

    return String(value.c_str());
}

void WebServerSensorController::_beginAtomicSendToClients()
{
    if (_groupUpdateInProgress) {
        Log.errorln("WebServerSensorController: group update begins while another is in progress!");
    }

    else {
        _groupUpdateInProgress = true;
    }
}

void WebServerSensorController::_sendToClients(const std::string& message) {
    _webSocketSendAll(message.c_str());
}

void WebServerSensorController::_endAtomicSendToClients() 
{
    if (!_groupUpdateInProgress) {
        Log.errorln("WebServerSensorController: group update ends but was never begun!");
    }
    
    else {
        _webSocketSendAll(_builder.str());
        _builder.clear();
        _groupUpdateInProgress = false;
    }
}    
    
int WebServerSensorController::_rssiToQuality(long dBm)
{
    int quality = 0;

    if(dBm <= -100)
        quality = 0;

    else if(dBm >= -50)
        quality = 100;

    else
        quality = 2 * (dBm + 100);

    return quality;
}
 
void WebServerSensorController::_webSocketSendAll(const std::string& message)
{
    _ScopedLock lock(_wsMutex);

    // Log.verboseln("WebServerSensorController: ws send \"%s\".", message);

    // The underlying library websocket implementation is problematic. 
    // If you fill up the queue it disconnects. And as a reconnection
    // means sending lots of upfront messages, we end up doing the same
    // disconnects over-and-over. So...
    //
    // We could mitigate in at least two possible ways:
    // (a) we don't send if we can detect that the queue is full.
    // (b) When possible, we use message aggregation.
    // However, we cannot detect if the queue is full because there
    // may be many (client) queues behind a textAll() call.
    // Hence we can at least easily mitigate using (b) above.

    // _webSocketSendTimer;
    // std::vector<std::string> _webSocketSendQueue; // See _webSocketSend for why this is necessary.

    // NOTE: now that we have *explicitly* aggregated messages, this deferred sending
    // logic is probably not required.

#if 1

    // static int totalMessagesSent = 0;
    if (_ws.availableForWriteAll()) {
        _ws.textAll(message.c_str());
    }

    else {
        Log.warningln("WebServerSensorController: failed to send to all web sockets - one or more queues were full.");
    }

    // Log.verboseln("WebServerLightsController: #### total sent = %d.", ++totalMessagesSent);

#else

    // Log.verboseln("WebServerLightsController: for ws, queueing \"%s\".", message.c_str());

    _webSocketSendQueue.push(message);

    if (_webSocketSendTimer.hasExpired())
    {
        _webSocketSendTimer.restart();

        size_t noOfMessages(_webSocketSendQueue.size());
        // Log.verboseln("WebServerLightsController: for ws, sending (aggregated) %d messages.", noOfMessages);

        if (noOfMessages > 0) 
        {
            std::stringstream ss;
            bool first(true);

            for (size_t i = 0; i < noOfMessages; ++i) {
                if (!first) ss << _messageDelimiter;
                first = false;
                ss << _webSocketSendQueue.front();
                _webSocketSendQueue.pop();
            }

            _ws.textAll(ss.str().c_str());
        }
    }
#endif
}

#if defined(ONBOARDING_HERE)

void WebServerSensorController::_saveWiFiCredentials(const std::string& ssid, const std::string& password) {
    _systemSettings.setWifiSSID(ssid);
    _systemSettings.setWifiPSK(password);
}

#endif

/*
void WebServerLightsController::_uiHandlePageRequest(AsyncWebServerRequest *request) 
{
    Log.verboseln("WebServerLightsController:_uiHandleControlBasic(...) url is %s", request->url());
    request->send(SPIFFS, "/ui/control/basic/index.html", "text/html", false);
}
*/

uint8_t WebServerSensorController::_forceToByte(long value) {
    long forced((value < 0) ? 0 : (value > 255) ? 255 : value);
    return (uint8_t)forced;
}

std::string WebServerSensorController::_normalised(const std::string& str) {
    // TODO: eradicate use of String.
    String result(str.c_str());
    result.trim();
    result.toLowerCase();
    return std::string(result.c_str());
}

bool WebServerSensorController::_split(const std::string& str, char delimiter, std::vector<std::string>& parts, bool ignoreEmpty) {
    std::vector<std::string> vec(parts.size());
    return Splitter::split(std::string(str.c_str()), std::back_inserter(parts), delimiter, ignoreEmpty) > 0;
}

std::string WebServerSensorController::_encodeSpaces(const std::string& str) {
    std::string encoded(str);
    std::replace(encoded.begin(), encoded.end(), ' ', '_');
    return encoded;
}

std::string WebServerSensorController::_decodeSpaces(const std::string& str) {
    std::string decoded(str);
    std::replace(decoded.begin(), decoded.end(), '_', ' ');
    return decoded;
}

#if 0

std::string WebServerSensorController::_asHex(const CRGB& rgb) {
    return _asHex(rgb.r, rgb.g, rgb.b);
}

std::string WebServerSensorController::_asHex(const CHSV& hsv) {
    return _asHex(hsv.h, hsv.s, hsv.v);
}

std::string WebServerSensorController::_asHex(uint8_t a, uint8_t b, uint8_t c) {
    std::ostringstream os;
    os << "#"         
        << std::setw(2) << std::setfill('0') << std::hex << (int)a
        << std::setw(2) << std::setfill('0') << std::hex << (int)b
        << std::setw(2) << std::setfill('0') << std::hex << (int)c;
    return os.str();
}

#endif

std::string WebServerSensorController::_toString(int value) {
    std::ostringstream os;
    os << (int)value;
    return os.str();
}

std::string WebServerSensorController::_toBoolString(bool state) {
    std::ostringstream os;
    os << (state ? "true" : "false");
    return os.str();
}

std::string WebServerSensorController::_toString(const IPAddress& address) {
    return std::string(address.toString().c_str());
}

std::string WebServerSensorController::_toString(float value, int precision) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(1) << value;
    return os.str();
}

bool WebServerSensorController::_startsWith(const std::string& str, const std::string& prefix) {
    String oldStr(str.c_str());
    return oldStr.startsWith(prefix.c_str());
}

void WebServerSensorController::_toLower(std::string& str) {
    // str = std::string(String(str.c_str()).toLower()));
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
}

int WebServerSensorController::_toInt(const std::string& value) {
    return String(value.c_str()).toInt();
}

uint8_t WebServerSensorController::_fullByteToPercentage(uint8_t val) {
    return map(val, 0, 255, 0, 100);
}

bool WebServerSensorController::_validIP(const std::string& ipStr) {
    IPAddress ipAddress;
    return ipAddress.fromString(ipStr.c_str());
}

std::string WebServerSensorController::_paramEncode(const std::string& param) {
    return _replaceAll(param, " ", "%20");
}

std::string WebServerSensorController::_replaceAll(const std::string& str, const std::string& substr, const std::string& replacement)
{
    std::ostringstream ss;

    size_t start = 0;
    size_t pos = str.find(substr, start);

    while (pos != std::string::npos) {
        ss << str.substr(start, pos-1);
        start = pos + substr.size();
        pos = str.find(substr, start);
    }

    return ss.str();
}

void WebServerSensorController::_trim(std::string& str) {
    static const char* whitespaces = " \t";    
    str.erase(str.find_last_not_of(whitespaces) + 1);
    str.erase(0,str.find_first_not_of(whitespaces));
}
