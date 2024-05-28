#pragma once

#if 1

#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>

#include "Chronos/TimeSpan.h"
#include "SmartSensorController.h"
#include "SmartSensorObserver.h"
//#include "UserSettings.h"

#include "Diagnostics/Logging.h"


#include "MQTTConnection.h"

class MQTTSensorController : public SmartSensorController, public SmartSensorObserver
{
public:

    typedef MQTTConnection::Topic Topic;
    typedef MQTTConnection::WildcardMatches WildcardMatches;
    
    enum class Format {
        Plain,
        JSON
    };

    MQTTSensorController(
        SmartSensor& smartSensor,
        // UserSettings& userSettings,
        const TimeSpan& loopInterval,
        MQTTConnection& mqtt,
        Format format = Format::Plain
    ) : SmartSensorController(smartSensor, "mqtt-controller", loopInterval)
      , SmartSensorObserver("mqtt-controller") 
      , _sensorTopicPrefix("")
      , _mqtt(mqtt)
      , _format(format)
      , _retainState(true)
    {}

    void setTopicPrefix(const Topic& topicPrefix) {
        _sensorTopicPrefix = topicPrefix;
    }

    inline bool isConnected() __attribute__((always_inline)) {
        return _mqtt.isConnected();
    }

    // Observer callbacks triggering outgoing messages.

    void onSwitchOnOff(bool on) override {
        _publishEntity("power", on);
    }

    void onFanSpeed(int speed) override {
        _publishEntity("fan", speed);
    }

    void onBacklightBrightness(uint8_t brightness) override {

    }

    void onBacklightColour(uint8_t hue, uint8_t sat) override {

    }

    void onTemperature(float temperature) override {
        _publishEntity("temperature", _toPayload(temperature, 1));
    }

    void onHumidity(float humidity) override {
        _publishEntity("humidity", _toPayload(humidity, 1));
    }

    void onAirPressure(float hPa) override {
        _publishEntity("pressure", (int)hPa);
    }

    void onTVOC(float tvoc) override {
        _publishEntity("TVOC", (int)tvoc);
    }

    void onCO2(float co2) override {
        _publishEntity("eCO2", (int)co2);
    }

    void onIAQAvailability(bool available) override {
        // TODO: Is this worth pubishing?
    }

    void onIAQ(float iaq) override {
        _publishEntity("IAQ", (int)iaq);
    }

private:

    void _initInputs() override
    {
        _mqtt.setup();

        _mqtt.subscribe(_sensorTopicPrefix + _Topic("power/set"), [this](const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload) -> bool {
            return _handlePowerOnOffMessage(topic, wildcardMatches, payload);
        });

        _mqtt.subscribe(_sensorTopicPrefix + _Topic("fan/set"), [this](const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload) -> bool {
            return _handleSetFanSpeedMessage(topic, wildcardMatches, payload);
        });

        _mqtt.subscribe(_sensorTopicPrefix + _Topic("fan-mode/set"), [this](const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload) -> bool {
            return _handleSetFanModeMessage(topic, wildcardMatches, payload);
        });
    }

    void _serviceInputs() override {
        _mqtt.loop();
    }

    void _publishEntity(const char* entityName, bool state) {
        _publishEntity(entityName, _toPayload(state));
    }

    void _publishEntity(const char* entityName, int value) {
        _publishEntity(entityName, _toPayload(value));
    }

    void _publishEntity(const char* entityName, const std::string& payload) {
        _mqtt.publish(_sensorTopicPrefix + _Topic(entityName), _terminate(payload), _retainState);
    }

    typedef MQTTConnection::Topic _Topic;

    /**
     *
     * MQTT Topic:
     * 
     * Topic prefix:
     *      eRora/sense/<sensor-host-name>
     * 
     * Subtopics (control):  RX
     *      <prefix>/power/set
     *      <prefix>/fan/set     0-10 (0 means totally off, 10 means full speed).
     *      ...etc...
     * 
     * Subtopics (status):  TX
     *      <prefix/power
     *      <prefix>/temperature        
     *      <prefix>/humidity
     *      <prefix>/pressure
     *      <prefix>/tvoc
     *      <prefix>/co2
     *      <prefix>/iaq
     *      <prefix/fan
     * 
     **/

    bool _handlePowerOnOffMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload)
    {
        bool parsed(true);

        if (_meansTrue(payload)) {
            _switchOnOff(true);
            // _publish(_onOffTopic, true);
        }

        else if (_meansFalse(payload)) {
            _switchOnOff(false);
            // _publish(_onOffTopic, false);
        }

        else if (payload == "toggle") {
            _toggleOnOff();
        }

        else {
            Log.warningln("MQTTLightSController: expected on/off/toggle for power on/off message but got \"%s\".", payload.c_str());
            parsed = false;
        }

        return parsed;
    }

    bool _handleSetFanSpeedMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload)
    {
        return false;
    }

    bool _handleSetFanModeMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload)
    {
        return false;
    }

    bool _handleDisplayModeMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload)
    {
        return false;
    }

    std::string _toPayload(float value, int precision = 1) {
        return (_format == Format::JSON) ? _toJsonPayload(value, precision) : _toPlainPayload(value, precision);
    }

    std::string _toPayload(int value) {
        return (_format == Format::JSON) ? _toJsonPayload(value) : _toPlainPayload(value);
    }

    std::string _toPayload(bool state) {
        return (_format == Format::JSON) ? _toJsonPayload(state) : _toPlainPayload(state);
    }

    std::string _toPlainPayload(float value, int precision = 1) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(precision) << value;
        return ss.str();
    }

    std::string _toPlainPayload(int value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }

    std::string _toPlainPayload(bool state) {
        std::stringstream ss;
        ss << (state ? "true" : "false");
        return ss.str();
    }
    
    std::string _toJsonPayload(float value, int precision = 1) {
        std::stringstream ss;
        ss << "{ \"value\": "<< std::fixed << std::setprecision(precision) << value << ")";
        return ss.str();
    }

    std::string _toJsonPayload(int value) {
        std::stringstream ss;
        ss << "{ \"value\": " << value << "}";
        return ss.str();
    }

    std::string _toJsonPayload(bool state) {
        std::stringstream ss;
        ss << "{ \"state\": " << (state ? "true" : "false") << "}";
        return ss.str();
    }
    
    std::string _terminate(const std::string& payload) {
        std::stringstream ss;
        ss << payload << std::endl;
        return ss.str();
    }

    static std::vector<std::string> _split(std::string argsStr, char delimiter);

    // UserSettings& _userSettings;

    Topic _sensorTopicPrefix;
    MQTTConnection& _mqtt;
    Format _format;
    bool _retainState;
};

#endif
