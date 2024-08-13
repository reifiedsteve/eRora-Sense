#pragma once

#if 1

#define ERORA_SENSE_SUPPORTS_SENSOR
#define ERORA_SENSE_SUPPORTS_BUTTONS
#define ERORA_SENSE_SUPPORTS_DISPLAY
#define ERORA_SENSE_SUPPORTS_FAN
#define ERORA_SENSE_SUPPORTS_MQTT

#include <WiFiClient.h>

#include "SmartSensor.h"
#include "PanelDisplay.h"

#include "MQTTConnection.h"
#include "MQTTSensorController.h"

#include "Controllers/WebServerSensorController.h"
#include "Chronos/Stopwatch.h"

#include "Discovery/DeviceExplorer.h"
#include "Diagnostics/OnboardLEDBlinker.h"
#include "ActivityLight.h"

#include "SystemSettings.h"
#include "UserSettings.h"

#include "Fan/PWMFanController.h"
#include "CabinetLights.h"

#include "Drivers/MomentaryButton.h"

#ifdef ERORA_SENSE_SUPPORTS_BUTTONS
#include "Controllers/ButtonController.h"
#endif

class MyEroraSensor
{
public:

    MyEroraSensor(OnboardLEDBlinker& blinker);

    void setup();
    void loop();

private:

    typedef MomentaryButton _Button;
    const int _buttonPollingInterval = 10; // milliseconds.

    const int _mqttPollingInterval = 100; // milliseconds.
    const int _webControllerPollingInterval = 10; // milliseconds.  // interval for propagating from (async) web client to our light state.

    void _initWiFi();
    bool _wifiConnect();

    static void _onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

    void _startAPMode();

    #ifdef ERORA_SENSE_SUPPORTS_MQTT
    void _initMQTTController();
    std::string _getMQTTTopicPrefix();
    std::string _getDefaultMQTTSensorTopicPrefix();
    #endif

    void _registerObservers();

    OnboardLEDBlinker& _blinker;
    ActivityLight _activityLight;

    std::string _deviceHostName;

    SystemSettings _systemSettings;
    UserSettings _userSettings;    
    
    WiFiClient _wifiClient;

    #ifdef ERORA_SENSE_SUPPORTS_MQTT
    MQTTConnection _mqttClient;
    #endif

    #ifdef ERORA_SENSE_SUPPORTS_SENSOR
    SmartSensor _smartSensor;
    #endif

    #ifdef ERORA_SENSE_SUPPORTS_DISPLAY
    PanelDisplay _display;
    #endif

    WebServerSensorController _webController;

    #ifdef ERORA_SENSE_SUPPORTS_MQTT
    MQTTSensorController _mqttController;
    #endif

    _Button _bootButton;

    #ifdef ERORA_SENSE_SUPPORTS_BUTTONS
    _Button _button1, _button2, _button3, _button4;
    ButtonController _buttonController;
    #endif

    #ifdef ERORA_SENSE_SUPPORTS_FAN
    uint8_t _fanPWMPinNo;
    PWMFanController _fan;
    #endif

    CabinetLights _cabinetLights;
    
    DeviceExplorer* _discoveryService; 
};

#endif
