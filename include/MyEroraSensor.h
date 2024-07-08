#pragma once

#if 1

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

#include "Controllers/ButtonController.h"
#include "Drivers/MomentaryButton.h"

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

    void _initMQTTController();
    std::string _getMQTTTopicPrefix();
    std::string _getDefaultMQTTSensorTopicPrefix();
    void _registerObservers();

    OnboardLEDBlinker& _blinker;
    ActivityLight _activityLight;

    std::string _deviceHostName;

    SystemSettings _systemSettings;
    UserSettings _userSettings;    
    
    WiFiClient _wifiClient;
    MQTTConnection _mqttClient;

    SmartSensor _smartSensor;

    PanelDisplay _display;

    WebServerSensorController _webController;
    MQTTSensorController _mqttController;

    _Button _button1, _button2, _button3, _button4;
    ButtonController _buttonController;

    uint8_t _fanPWMPinNo;
    PWMFanController _fan;

    CabinetLights _cabinetLights;
    
    DeviceExplorer* _discoveryService; 
};

#endif
