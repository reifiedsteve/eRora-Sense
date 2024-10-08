#include <NetBIOS.h>

#include "MyEroraSensor.h"
#include "MACAddress.h"
#include "PinAssignments.h"

MyEroraSensor::MyEroraSensor(OnboardLEDBlinker& blinker)
    : _blinker(blinker)
    , _activityLight(_blinker)
    , _deviceHostName()
    , _systemSettings()
    , _userSettings("user")
    , _wifiClient()
    #ifdef ERORA_SENSE_SUPPORTS_MQTT
    , _mqttClient(
        _wifiClient, 
        "192.168.1.186", 
        1883,
        "mqtt",
        "mqtt"
    )
    #endif
    , _smartSensor()
    #ifdef ERORA_SENSE_SUPPORTS_DISPLAY
    , _display()
    #endif
    , _webController(
        _smartSensor, 
        _systemSettings,
        _userSettings,
        TimeSpan::fromMilliseconds(_webControllerPollingInterval),
        80
    )
    #ifdef ERORA_SENSE_SUPPORTS_MQTT
    , _mqttController(
        _smartSensor, 
        TimeSpan::fromMilliseconds(_mqttPollingInterval),
        _mqttClient
    )
    #endif
    , _bootButton(PinAssignments::OnboardBootButton)
    #ifdef ERORA_SENSE_SUPPORTS_BUTTONS
    , _button1(PinAssignments::Button1)
    , _button2(PinAssignments::Button2)
    , _button3(PinAssignments::Button3)
    , _button4(PinAssignments::Button4)
    , _buttonController(
        _smartSensor,
        TimeSpan::fromMilliseconds(10),
        _button1,
        _button2,
        _button3,
        _button4
    )
    #endif
    #ifdef ERORA_SENSE_SUPPORTS_FAN
    , _fanPWMPinNo(PinAssignments::FanPWM)
    , _fan(_fanPWMPinNo)
    #endif
    , _cabinetLights(_systemSettings.getNumberOfLEDs())
{}

void MyEroraSensor::setup()
{
    // Use default i2c pins (SDA:21, SCL:22),
    // Bus speed: Default is 400,000. 
    // 10Khz is fast enough but more reliable with longer wires.
    // However, BME680 requires minimum of 100KHz.
    Wire.setPins(SDA, SCL);
    Wire.setClock(100000); 
    
    delay(500);

    #ifdef ERORA_SENSE_SUPPORTS_DISPLAY
    _display.setup();
    delay(1000);
    #endif

    Log.verboseln("About to initialise WiFi...");
    _initWiFi();
    Log.verboseln("Initialised WiFi");
    delay(500);

    Log.infoln("MAC is %s.", MACAddress::local().str(":").c_str());
    // Log.infoln("Hostname is %s", _getDefaultDeviceHostName().c_str());
    Log.infoln("Hostname is %s", _systemSettings.getDeviceName().c_str());

    #ifdef ERORA_SENSE_SUPPORTS_FAN
    _fan.configSeparatePowerControlPin(PinAssignments::FanPowerControl);
    // _fanController.limitPhysicalSpeedRange(0, 100);
    _fan.begin();
    #endif

    #ifdef ERORA_SENSE_SUPPORTS_FAN
    _fan.setSpeed(100);
    _fan.setPower(true);
    _smartSensor.bindFanController(_fan);
    #endif
    
    _cabinetLights.setup();

    _cabinetLights.setCurrentLimit(_systemSettings.getLEDsPSUMilliamps());
    _cabinetLights.setMaximumBrightness(_userSettings.getCabinetLightBrightness());
    _cabinetLights.setAmbientColour(_userSettings.getCabinetLightColour());
    _cabinetLights.setInspectionColour(_userSettings.getCabinetInspectionLightColour());
    _cabinetLights.setInspectionAutoOffTime(_userSettings.getInspectionTime());
    _smartSensor.bindCabinetLights(_cabinetLights);
    
    #ifdef ERORA_SENSE_SUPPORTS_MQTT
    _initMQTTController();
    #endif

    static const unsigned clickMs(200);
    static const unsigned pressMs(500);
    static const unsigned debounceMs(50);

    _bootButton.setClickMs(clickMs);
    _bootButton.setDebounceMs(debounceMs);
    _bootButton.setPressMs(pressMs);

    _bootButton.attachClick([this]() {
        _blinker.start(3, 250);
        _startAPMode();
    });

    #ifdef ERORA_SENSE_SUPPORTS_BUTTONS

    _button1.setClickMs(clickMs);
    _button1.setDebounceMs(debounceMs);
    _button1.setPressMs(pressMs);
    _button2.setClickMs(clickMs);
    _button2.setDebounceMs(debounceMs);
    _button2.setPressMs(pressMs);
    _button3.setClickMs(clickMs);
    _button3.setDebounceMs(debounceMs);
    _button3.setPressMs(pressMs);
    _button4.setClickMs(clickMs);
    _button4.setDebounceMs(debounceMs);
    _button4.setPressMs(pressMs);

    _buttonController.setup();

    #endif

    _webController.setup();
    
    delay(500);

    _smartSensor.setup();
    delay(500);

    _registerObservers();
    delay(500);

    _discoveryService = new DeviceExplorer(
        DeviceCategory::MultiSensor,
        _systemSettings.getDeviceDescriptiveName(), 
        0, // #LEDS !!!! TODO: fix this!
        5000, 60000
    );

    _discoveryService->registerForChanges(
        // Maybe the smart light core logic should be told about these events, but
        // for now can't think of a good reason why. So (at least for now) 
        // let's keep it out of the picture as only the web controller
        // currently needs to know.
        [this](const DeviceInformation& deviceDetails, DeviceExplorer::DeviceEvent event) {
            switch (event) {
                case DeviceExplorer::DeviceEvent::ComeOnline:
                    _webController.addDevice(deviceDetails);
                    break;
                case DeviceExplorer::DeviceEvent::Modified:
                    _webController.removeDevice(deviceDetails);
                    _webController.addDevice(deviceDetails);
                    break;
                case DeviceExplorer::DeviceEvent::GoneOffline:
                    _webController.removeDevice(deviceDetails);
                    break;
                default:
                    break;

            }
        },
        false // No point in telling us all it knows immediately - as it's not setup() yet (see below).
    );

    _discoveryService->setup();
}

void MyEroraSensor::loop()
{
    _bootButton.tick();
    
    #ifdef ERORA_SENSE_SUPPORTS_BUTTONS
    _buttonController.loop();
    #endif

    _webController.loop();

    #ifdef ERORA_SENSE_SUPPORTS_MQTT
    _mqttController.loop();
    #endif

    _smartSensor.loop();

    _cabinetLights.loop();

    #ifdef ERORA_SENSE_SUPPORTS_DISPLAY
    _display.loop();
    #endif

    _discoveryService->loop();
}

void MyEroraSensor::_initWiFi()
{
    // Do we really need to disable WiFi sleep? 
    // Doing so uses a little more power (using 145ma
    // as opposed to when allowing WiFi to sleep with 80ma-108mA)
    // but might improve respsonsiveness.
    // However, as power consumption is minimal anyway, the
    // difference in running costs is around £0.30 per year
    // if always plugged in.

    WiFi.setSleep(false);

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        _onWiFiEvent(event, info);
    });

    WiFi.mode(WIFI_STA);

    if (_systemSettings.useManualNetworkConfig()) {
        WiFi.config(
            _systemSettings.getStaticIP(),
            _systemSettings.getGateway(),
            _systemSettings.getSubnet(),
            _systemSettings.getDNS1(),
            _systemSettings.getDNS2()
        );
    }

    _deviceHostName = _systemSettings.getDeviceName();
    Log.infoln("MyEroraLight: device host name is \"%s\".", _deviceHostName.c_str());

    WiFi.setHostname(_deviceHostName.c_str());

    std::string wifiSsid(_systemSettings.getWifiSSID());
    std::string wifiPsk(_systemSettings.getWifiPSK());
    
    Log.verboseln("MyEroraLight: WiFi ssid \"%s\", psk \"%s\"", wifiSsid.c_str(), wifiPsk.c_str());
    
    WiFi.begin(wifiSsid.c_str(), wifiPsk.c_str()); /* Connect to Wi-Fi based on given SSID and Password */
    
    bool connected(_wifiConnect());

    if (connected) { // TODO: should this even be conditional?
        //_display.onWifiSSID(std::string(WiFi.SSID().c_str()));     // TODO: don't drive display directly! Tell *all* observers.
        //_display.onIPAddress(WiFi.localIP());
    }

    else {
        //_display.onNotification("Started as AP.");
        _startAPMode();
    }

    NBNS.begin(_deviceHostName.c_str()); // Allow device to be found by name (as well as by IP) for convenience.

    delay(2000);
}

bool MyEroraSensor::_wifiConnect()
{
    CountdownTimer::Ticks connectionTimeout(30000); // 15 secs.
    CountdownTimer wifiConnectTimer(connectionTimeout);

    wifiConnectTimer.start();

    Log.infoln("MyEroraSensor: connecting to wifi...");

    WiFi.setAutoConnect(true);
    
    wl_status_t status(WiFi.status());

    while ((status != WL_CONNECTED) && !wifiConnectTimer.hasExpired()) {
        Log.infoln("MyEroraSensor: still connecting to wifi (status %d)...", (int)status);
        delay(1000);
        status = WiFi.status();
    }

    bool connected(WiFi.status() == WL_CONNECTED);

    if (connected) {
        Log.infoln("MyEroraSensor: Wifi connected (SSID: \"%s\")", WiFi.SSID().c_str());
    } else {
        Log.errorln("MyEroraSensor: Wifi not connected");
    }

    return connected;
}

void MyEroraSensor::_onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
    switch (event)
    {
        case SYSTEM_EVENT_STA_CONNECTED:
            Log.infoln("WIFI: Connected to WiFi Network (LAN)");
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            Log.infoln("WIFI: Disconnected from WiFi Network (LAN)");
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            Log.infoln("WIFI: LAN assigned IP: %s", WiFi.localIP().toString().c_str());
            break;

        case SYSTEM_EVENT_AP_START:
            Log.infoln("WIFI: Access-point started and awaiting clients");
            break;

        case SYSTEM_EVENT_AP_STOP:
            Log.infoln("WIFI: Access-point stopped");
            break;

        case SYSTEM_EVENT_AP_STAIPASSIGNED:
            Log.infoln("WIFI: Access-point IP:", WiFi.softAPIP().toString().c_str());
            break;

        case SYSTEM_EVENT_AP_STACONNECTED:
            Log.infoln("WIFI: Access-point accepted new remote client");
            break;

        case SYSTEM_EVENT_AP_STADISCONNECTED:
            Log.infoln("WIFI: Access-point had remote client disconnect.");
            break;

        default: break;
    }
}

void MyEroraSensor::_startAPMode()
{
    WiFi.mode(WIFI_AP_STA);

    // std::string deviceHostName(_getDefaultDeviceHostName());
    std::string deviceHostName(_systemSettings.getDeviceName());

    const char* ssid = deviceHostName.c_str();
    const char* psk = "LightMeUp";
    const int channelNo = 1;
    const int hiddenSSid = 0; // Do not hide that SSID as that would defeat the objective.
    const int maxClients = 1; // Limit to just the user doing the set-up for added security.

    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,9), IPAddress(255,255,255,0));
    WiFi.softAP(ssid, psk, channelNo, hiddenSSid, maxClients);

    IPAddress apAddr(WiFi.softAPIP());

    #if 0
    std::ostringstream ss;
    // ss << "WiFi on " << apAddr.toString().c_str();
    // _display.onNotification(String(ss.str().c_str()), 60000U);    // TODO: also show SSID, PSK.
    ss << "Set-up active.";
    _display.refresh();
    #endif
}

#ifdef ERORA_SENSE_SUPPORTS_MQTT

void MyEroraSensor::_initMQTTController() {
    std::string topicPrefix(_getMQTTTopicPrefix());
    Log.infoln("MQTT topic prefix is \"%s\"", topicPrefix.c_str());
    _mqttController.setTopicPrefix(MQTTTopic(topicPrefix));
    _mqttClient.setup();
    _mqttController.setup();
}

std::string MyEroraSensor::_getMQTTTopicPrefix() {
    std::string prefix(_systemSettings.getMQTTTopicPrefix());
    if (prefix == "") prefix = _getDefaultMQTTSensorTopicPrefix();
    return prefix;
}

std::string MyEroraSensor::_getDefaultMQTTSensorTopicPrefix() {
    std::string prefix("eRora/sensors/");
    prefix += MACAddress::local().str().substr(6) + "/"; // Just use hex chars from last 3 MAC bytes.
    return prefix;
}

#endif

void MyEroraSensor::_registerObservers()
{
    #ifdef ERORA_SENSE_SUPPORTS_DISPLAY
    _smartSensor.bindObserver(_display);
    #endif

    _smartSensor.bindObserver(_webController);

    #ifdef ERORA_SENSE_SUPPORTS_MQTT
    _smartSensor.bindObserver(_mqttController);
    #endif

    // _smartSensor.bindStateObserver(_webController);

    #ifdef USE_THREE_BUTTON_CONTROLLER
    _smartSensor.bindStateObserver(_threeButtonController);
    #endif

    _smartSensor.bindObserver(_activityLight);
}







