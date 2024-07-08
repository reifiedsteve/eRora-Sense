#pragma once

// Have the async TCP and server running on the same core as the WiFi stack etc.
// We want to avoid it being on the same core as FastLED as FastLED
// can suffer glitches if interrupted by other tasks (as the data
// waveform generation gets corrupted).
// Note: Core 1 is the default (application) core. Core 0 is the
// 2nd core that runs the WiFi stack etc.).

// #define CONFIG_ASYNC_TCP_RUNNING_CORE 0 // -1 for any available core
// TODO: decide on  which core to use. (Using -1 seems less visually glitchy on the lights.)

#include <sstream>

#include <DNSServer.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include "SmartSensorController.h"
#include "SmartSensorObserver.h"

#include "Chronos/Stopwatch.h"
#include "Chronos/TimeSpan.h"

#include "Diagnostics/Logging.h"

#include "Discovery/DeviceInformation.h"

#include "SystemSettings.h"

#include "UserSettings.h"

//#include "ScheduledActions.h"

#include "DeviceState.h"

#include "Utils/LabelledValues.h"
#include "Utils/DelimitedTextBuilder.h"

#include <queue>
#include <string>
#include <mutex>

#define ONBOARDING_HERE

#if defined(ONBOARDING_HERE)

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler();
  virtual ~CaptiveRequestHandler();

  bool canHandle(AsyncWebServerRequest *request);
  void handleRequest(AsyncWebServerRequest *request);
};

# endif

class WebServerSensorController
  : public SmartSensorController
  , public SmartSensorObserver
{
public:

    typedef TimeSpan::Ticks Ticks;

    /// @brief Contruct a web lights controller.
    /// @brief smartLight The smart light to be controlled.
    /// @param leds The LEDs state for web-preview purposes.
    /// @param portNo The port from which the web server is accessed.
    WebServerSensorController(
        SmartSensor& smartSensor,
        SystemSettings& systemSettings,
        UserSettings& userSettings,
        const TimeSpan& loopInterval,
        int portNo
    );

    void setDevicesInfo(const String& deviceInfoStr);

    /// @brief Provide the UI with the names of all effects so that it can present
    /// them to the user. 
    /// @param effectNamesStr A string consisting of all effect names, each delineated
    /// from the others by a semicolon.
    void setEffectsNames(const String& effectNamesStr);

    void setNumberOfUserPresets(uint8_t maxUserPresets, uint8_t userPresetsBase = 1);
    void setNumberOfNotifications(uint8_t maxNotifications);

    // TODO...

    // void onObservationStarts(const SmartSensor& smartSensor) override;

#if 0

    void onAPSSID(const std::string& ssid) override;

    void onWifiSSID(const std::string& ssid) override;
    void onSignalStrength(long strength) override;

    void onIPAddress(const IPAddress& ip) override;
    void onHostName(const std::string& hostName) override;   // ...and inferred http:// URL
    void onMAC(const MACAddress& mac) override;

    void onProgressStart(const std::string& processName) override;
    void onProgress(uint8_t percentageComplete, const std::string& progressMessage) override;
    void onProgressEnd() override;

#endif

    // Fan controlling being observed.

    void onSwitchOnOff(bool on) override;
    void onFanSpeed(int speed) override;

    void onCabinetBrightness(uint8_t brightness) override;
    void onCabinetColour(uint8_t hue, uint8_t sat) override;

    // Sensor measurements being observed.

    void onTemperature(float temperature) override;
    void onHumidity(float humidity) override;
    void onAirPressure(float hPa) override;
    void onTVOC(float tvoc) override;
    void onCO2(float co2) override;

    void onIAQAvailability(bool available) override;
    void onIAQ(float iaq) override;

    void onPM01(uint16_t pm01) override;
    void onPM25(uint16_t pm25) override;
    void onPM10(uint16_t pm10) override;
    
    // General MCU health being observed.

    void onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) override;
    void onFPS(unsigned fps) override;

    // We need to know when a block update is happening so that 
    // we can optimise our sending of this data over web sockets
    // to client browsers. This is because the underlying 
    // libraries do not like to be flooded with too many WS
    // send requests at once.

    virtual void onGroupUpdateBegins() override {
        _beginAtomicSendToClients();
    }

    virtual void onGroupUpdateEnds() override {
        _endAtomicSendToClients();
    }

    // Specific to web-controller rather than a sensor-observer
    // (but that might need to change...?).

    void addDevice(const DeviceInformation& deviceDetails);
    void removeDevice(const DeviceInformation& deviceDetails);
    
private:

    typedef std::list<DeviceInformation> _Devices;
    typedef _Devices::const_iterator _DevicesConstIter;
    typedef _Devices::iterator _DevicesIter;

    typedef std::function<void ()> _UpdateFunc;
    typedef std::queue<_UpdateFunc> _GroupUpdateQueue;

    static const size_t _maxGroupUpdates = 32;

    static const char _messageDelimiter = '~';
    
    void _respondToSwitchOnOff(bool on);
    void _respondToFanSpeed(int speed);
    void _respondToCabinetBrightness(uint8_t brightness);
    void _respondToCabinetColour(uint8_t hue, uint8_t sat);
    void _respondToTemperature(float temperature);
    void _respondToHumidity(float humidity);
    void _respondToAirPressure(float hPa);
    void _respondToTVOC(float tvoc);
    void _respondToCO2(float co2);
    void _respondToIAQAvailability(bool available);
    void _respondToIAQ(float iaq);
    void _respondToPM01(uint16_t pm01);
    void _respondToPM25(uint16_t pm25);
    void _respondToPM10(uint16_t pm10);

    void _respondToHeapUsage(uint32_t totalHeap, uint32_t freeHeap);
    void _respondToFPS(unsigned fps);

    void _initInputs() override;
    void _serviceInputs() override;

    static void _makeFrameRateMessage(std::ostream& os, unsigned fps);
    static void _makeHeapUsageMessage(std::ostream& os, uint32_t totalHeap, uint32_t freeHeap);

    static std::string _makeAllDevicesMessage(const _Devices& devices);
    static void _makeAllDevicesMessage(std::ostream& os, const _Devices& devices);

    static void _makeAddDeviceMessage(std::ostream& os, const DeviceInformation& device);
    static void _makeAddDeviceMessage(std::ostream& os, const IPAddress& address, DeviceCategory deviceType, const std::string& deviceName);

    static void _makeRemoveDeviceMessage(std::ostream& os, const IPAddress& address);

    void _onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
    void _onWebSocketConnect(AsyncWebSocket* server, AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);
    void _onWebSocketDisconnect(AsyncWebSocket* server, AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);
    void _onWebSocketError(AsyncWebSocket* server, AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);
    void _onWebSocketPong(AsyncWebSocket* server, AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);
    void _onWebSocketMessage(AsyncWebSocket* server, AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);

    void _apiHandleRequest(AsyncWebServerRequest *request);
    bool _apiHandleStateQuery(AsyncWebServerRequest* request, const std::string& queryItemName);
    bool _apiHandleCommands(AsyncWebServerRequest* request);
    bool _apiHandleCommand(const std::string& command, const std::string& value);

    void _sendUserSettings(AsyncWebSocketClient* client);

    void _appendAllControlStates(std::ostream& os);
    void _appendAllSensorReadings(std::ostream& os);
    
    std::string _makePowerMessage(bool on);
    std::string _makeFanSpeedMessage(int speed);
    std::string _makeTemperatureMessage(float temperature);
    std::string _makeHumidityMessage(float humidity);
    std::string _makeAirPressureMessage(float pressure);
    std::string _makeTVOCMessage(float tvoc);
    std::string _makeCO2Message(float co2);
    std::string _makeIAQAvailabilityMessage(bool available);
    std::string _makeIAQMessage(float iaq);
    std::string _makePM01Message(uint16_t pm01);
    std::string _makePM25Message(uint16_t pm25);
    std::string _makePM10Message(uint16_t pm10);
    std::string _makeHeapUsageMessage(uint32_t totalHeap, uint32_t freeHeap);
    std::string _makeFPSMessage(unsigned fps);

    void _appendPowerMessage(std::ostream& os, bool on);
    void _appendFanSpeedMessage(std::ostream& os, int speed);
    void _appendTemperatureMessage(std::ostream& os, float temperature);
    void _appendHumidityMessage(std::ostream& os, float humidity);
    void _appendAirPressureMessage(std::ostream& os, float pressure);
    void _appendTVOCMessage(std::ostream& os, float tvoc);
    void _appendCO2Message(std::ostream& os, float co2);
    void _appendIAQAvailabilityMessage(std::ostream& os, bool available);
    void _appendIAQMessage(std::ostream& os, float iaq);
    void _appendPM01Message(std::ostream& os, uint16_t pm01);
    void _appendPM25Message(std::ostream& os, uint16_t pm25);
    void _appendPM10Message(std::ostream& os, uint16_t pm10);
    void _appendHeapUsageMessage(std::ostream& os, uint32_t totalHeap, uint32_t freeHeap);
    void _appendFPSMessage(std::ostream& os, unsigned fps);
 
    void _appendAllUserSettingsMessage(std::ostream& os);
    void _appendUserSettingMessage(std::ostream& os, const char* settingName, int value);
    void _appendUserSettingMessage(std::ostream& os, const char* settingName, unsigned value);
    void _appendUserSettingMessage(std::ostream& os, const char* settingName, bool state);
    void _appendUserSettingMessage(std::ostream& os, const char* settingName, const std::string& str);
    void _appendUserSettingMessage(std::ostream& os, const char* settingName, const char* str);

    /*
    void _sendLEDsState(CRGBView& leds);
    void _sendLEDsState(CRGB rgb, size_t count);
   
    std::string _makeLEDsStateMessage(CRGBView& leds);
    void _makeLEDsStateMessage(std::ostream& os, CRGBView& leds);

    std::string _makeLEDsStateMessage(CRGB rgb, size_t count);
    void _makeLEDsStateMessage(std::ostream& os, CRGB rgb, size_t count);
    */

    // Device control.

    void _parseRequest(const std::string& message);
    bool _parseSingleRequest(const std::string& message);

    bool _parsePowerRequest(const std::string& value);
    bool _parseFanSpeedRequest(const std::string& value);
    // bool _parseControlBrightnessRequest(const std::string& value, bool isPercentage);
    
    bool _parseUserSettingRequest(const std::string& settingName, const std::string& valueStr);

    /*
    bool _parseTimer(const std::string& timerNoStr, const std::string& timeStr, const std::string& daysStr, const std::string& actionStr);
    bool _parseTimer(const std::string& timerNoStr, const std::string& scheduledActionStr);
    */

    // Web UI.

    // void _uiHandleControlBasic(AsyncWebServerRequest *request);
    #if defined(ONBOARDING_HERE)
    void _uiHandleOnboarding(AsyncWebServerRequest *request);
    // void _uiHandleOnboardingPost(AsyncWebServerRequest *request, uint8_t* bytes, size_t len, size_t index, size_t total);
    #endif
    
    std::string _applySettings(const LabelledValues& settings);

    static LabelledValues _getParametersFromMethodGET(AsyncWebServerRequest *request);
    #if false
    static LabelledValues _getParametersFromMethodPOST(AsyncWebServerRequest *request, uint8_t* bytes, size_t len, size_t index, size_t total);
    #endif
    
    void _uiHandleControl(AsyncWebServerRequest *request);

    void _serveFromFile(AsyncWebServerRequest *request);
    String _processTemplates(const String& var);

    static int _rssiToQuality(long dBm);

    void _beginAtomicSendToClients();
    void _sendToClients(const std::string& message);
    void _endAtomicSendToClients();
    
    void _webSocketSendAll(const std::string& message);

    #if defined(ONBOARDING_HERE)
    void _saveWiFiCredentials(const std::string& ssid, const std::string& password);
    #endif

    static uint8_t _forceToByte(long value);

    static std::string _normalised(const std::string& str);
    static bool _split(const std::string& str, char delimiter, std::vector<std::string>& parts, bool ignoreEmpty = true);

    static std::string _encodeSpaces(const std::string& str);
    static std::string _decodeSpaces(const std::string& str);

    static std::string _toString(int value);
    static std::string _toBoolString(bool state);
    static std::string _toString(const IPAddress& address);
    static std::string _toString(float value, int precision);

    static bool _startsWith(const std::string& str, const std::string& prefix);
    static void _toLower(std::string& str);
    static int _toInt(const std::string& value);
    
    static uint8_t _fullByteToPercentage(uint8_t val);

    static bool _validIP(const std::string& ipStr);

    std::string _paramEncode(const std::string& param);

    static std::string _replaceAll(const std::string& str, const std::string& substr, const std::string& replacement);
    static void _trim(std::string& str);

    typedef Stopwatch _Stopwatch;
    typedef _Stopwatch::Ticks _Ticks;

    typedef std::mutex _Mutex;
    typedef std::lock_guard<_Mutex> _ScopedLock;

    typedef std::queue<std::string> _MessageQueue;

    SystemSettings& _systemSettings;
    UserSettings& _userSettings;
    
    AsyncWebServer _server;

    // ScheduledActions& _scheduledActions;

    AsyncWebSocket _ws;
    CountdownTimer _webSocketSendTimer;
    _MessageQueue _webSocketSendQueue; // See _wsSend for why this is necessary.

    CountdownTimer _wsCleanupTimer;

    mutable _Mutex _wsMutex;
    
    std::string _installationSSID;
    
    DeviceState _state;
    _Devices _allDevices; 

    bool _groupUpdateInProgress;

    DelimitedTextBuilder _builder;
    std::stringstream _wsOutputStream;
};
