#pragma once

#if 1

#include <WiFiClient.h>

#include "SmartSensor.h"
#include "PanelDisplay.h"

#include "MQTTConnection.h"
#include "MQTTSensorController.h"

#include "Chronos/Stopwatch.h"

// #include "Discovery/DeviceExplorer.h"
#include "Diagnostics/OnboardLEDBlinker.h"
#include "ActivityLight.h"

#include "SystemSettings.h"
#include "UserSettings.h"

class MyEroraSensor
{
public:

    MyEroraSensor(OnboardLEDBlinker& blinker);

    void setup();
    void loop();

private:

    void _initWiFi();
    bool _wifiConnect();

    static void _onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

    void _startAPMode();

    void _initMQTTController();
    std::string _getMQTTTopicPrefix();
    std::string _getDefaultMQTTLightTopicPrefix();
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

    MQTTSensorController _mqttController;    
};

#endif