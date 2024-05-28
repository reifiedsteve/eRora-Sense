#include <NetBIOS.h>

#include "MyEroraSensor.h"
#include "MACAddress.h"

MyEroraSensor::MyEroraSensor(OnboardLEDBlinker& blinker)
    : _blinker(blinker)
    , _activityLight(_blinker)
    , _deviceHostName()
    , _systemSettings()
    , _userSettings("user")
    , _wifiClient()
    , _mqttClient(
        _wifiClient, 
        "192.168.1.186", 
        1883,
        "mqtt",
        "mqtt"
    )
    , _smartSensor()
    , _display()
    , _mqttController(
        _smartSensor, 
        TimeSpan::fromMilliseconds(500),
        _mqttClient
    )
{}

void MyEroraSensor::setup()
{
    delay(500);
    _display.setup();
    delay(1000);

    Log.verboseln("About to initialise WiFi...");
    _initWiFi();
    Log.verboseln("Initialised WiFi");
    delay(500);

    Log.infoln("MAC is %s.", MACAddress::local().str(":").c_str());
    // Log.infoln("Hostname is %s", _getDefaultDeviceHostName().c_str());
    Log.infoln("Hostname is %s", _systemSettings.getDeviceName().c_str());

    _smartSensor.bindObserver(_display);
    delay(2000);

    _initMQTTController();
    delay(500);

    _registerObservers();
    delay(500);

    _smartSensor.setup();

    #if 1

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
                        //_webController.addDevice(deviceDetails);
                        break;
                    case DeviceExplorer::DeviceEvent::NameChange:
                        //_webController.removeDevice(deviceDetails);
                        //_webController.addDevice(deviceDetails);
                        break;
                    case DeviceExplorer::DeviceEvent::GoneOffline:
                        //_webController.removeDevice(deviceDetails);
                        break;
                    default:
                        break;

                }
            },
            false // No point in telling us all it knows immediately - as it's not setup() yet (see below).
        );

        _discoveryService->setup();

    #endif
}

void MyEroraSensor::loop()
{
    _display.loop();

    _smartSensor.loop();
    _mqttController.loop();

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

void MyEroraSensor::_registerObservers()
{
    _smartSensor.bindObserver(_mqttController);
    // _smartSensor.bindStateObserver(_webController);

    #ifdef USE_THREE_BUTTON_CONTROLLER
    _smartSensor.bindStateObserver(_threeButtonController);
    #endif

    _smartSensor.bindObserver(_activityLight);
}







