#include "Controllers/MQTTSensorController.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

#include "Diagnostics/Logging.h"
#include "Utils/Parser.h"

#include "MQTTConnection.h"

MQTTSensorController::MQTTSensorController(
    SmartSensor& smartSensor,
    // UserSettings& userSettings,
    const TimeSpan& loopInterval,
    MQTTConnection& mqtt,
    Format format
) : SmartSensorController(smartSensor, "mqtt-controller", loopInterval, Responsiveness::Always)
    , SmartSensorObserver("mqtt-controller") 
    , _sensorTopicPrefix("")
    , _mqtt(mqtt)
    , _format(format)
    , _retainState(true)
{}

void MQTTSensorController::setTopicPrefix(const Topic& topicPrefix) {
    _sensorTopicPrefix = topicPrefix;
}

bool MQTTSensorController::isConnected() {
    return _mqtt.isConnected();
}

// Observer callbacks triggering outgoing messages.

void MQTTSensorController::onSwitchOnOff(bool on) {
    _publishEntity("power", on);
}

void MQTTSensorController::onFanSpeed(int speed) {
    _publishEntity("fan", speed);
}

void  MQTTSensorController::onCabinetInspectionLightOnOff(bool on) {
    _publishEntity("inspect", on);
}

void MQTTSensorController::onCabinetBrightness(uint8_t brightness) {

}

void MQTTSensorController::onCabinetColour(uint8_t hue, uint8_t sat) {

}

void MQTTSensorController::onTemperature(float temperature) {
    _publishEntity("temperature", _toPayload(temperature, 1));
}

void MQTTSensorController::onHumidity(float humidity) {
    _publishEntity("humidity", _toPayload(humidity, 1));
}

void MQTTSensorController::onAirPressure(float hPa) {
    _publishEntity("pressure", (int)hPa);
}

void MQTTSensorController::onTVOC(float tvoc) {
    _publishEntity("TVOC", (int)tvoc);
}

void MQTTSensorController::onCO2(float co2) {
    _publishEntity("eCO2", (int)co2);
}

void MQTTSensorController::onIAQAvailability(bool available) {
    // TODO: Is this worth pubishing?
}

void MQTTSensorController::onIAQ(float iaq) {
    _publishEntity("IAQ", (int)iaq);
}

void MQTTSensorController::onPM01(uint16_t pm01) {
    _publishEntity("PM1", (int)pm01);
}

void MQTTSensorController::onPM25(uint16_t pm25) {
    _publishEntity("PM25", (int)pm25);
}

void MQTTSensorController::onPM10(uint16_t pm10) {
    _publishEntity("PM10", (int)pm10);
}

void MQTTSensorController::onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) {
    // Publish this?
}

void MQTTSensorController::_initInputs()
{
    _mqtt.setup();

    _mqtt.subscribe(_sensorTopicPrefix + _Topic("power/set"), [this](const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload) -> bool {
        return _handlePowerOnOffMessage(topic, wildcardMatches, payload);
    });

    _mqtt.subscribe(_sensorTopicPrefix + _Topic("inspect/set"), [this](const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload) -> bool {
        return _handleSetCabinetInspectionLightMessage(topic, wildcardMatches, payload);
    });

    _mqtt.subscribe(_sensorTopicPrefix + _Topic("fan/set"), [this](const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload) -> bool {
        return _handleSetFanSpeedMessage(topic, wildcardMatches, payload);
    });

    _mqtt.subscribe(_sensorTopicPrefix + _Topic("fan-mode/set"), [this](const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload) -> bool {
        return _handleSetFanModeMessage(topic, wildcardMatches, payload);
    });
}

void MQTTSensorController::_serviceInputs() {
    _mqtt.loop();
}

void MQTTSensorController::_publishEntity(const char* entityName, bool state) {
    _publishEntity(entityName, _toPayload(state));
}

void MQTTSensorController::_publishEntity(const char* entityName, int value) {
    _publishEntity(entityName, _toPayload(value));
}

void MQTTSensorController::_publishEntity(const char* entityName, const std::string& payload) {
    _mqtt.publish(_sensorTopicPrefix + _Topic(entityName), _terminate(payload), _retainState);
}

bool MQTTSensorController::_handlePowerOnOffMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload)
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

bool MQTTSensorController::_handleSetCabinetInspectionLightMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload)
{
    bool parsed(true);

    if (_meansTrue(payload)) {
        _triggerInspection();
    }

    else if (_meansFalse(payload)) {
        // Auto switch off only.
    }

    else {
        Log.warningln("MQTTLightSController: expected on/off for inspection on/off message but got \"%s\".", payload.c_str());
        parsed = false;
    }

    return parsed;
}

bool MQTTSensorController::_handleSetFanSpeedMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload) {
    int speed;
    bool parsed(Parser::tryParse(payload, speed));
    if (parsed) _setFanSpeed(speed);
    return parsed;
}

bool MQTTSensorController::_handleSetFanModeMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload)
{
    return false;
}

bool MQTTSensorController::_handleDisplayModeMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload)
{
    return false;
}

std::string MQTTSensorController::_toPayload(float value, int precision) {
    return (_format == Format::JSON) ? _toJsonPayload(value, precision) : _toPlainPayload(value, precision);
}

std::string MQTTSensorController::_toPayload(int value) {
    return (_format == Format::JSON) ? _toJsonPayload(value) : _toPlainPayload(value);
}

std::string MQTTSensorController::_toPayload(bool state) {
    return (_format == Format::JSON) ? _toJsonPayload(state) : _toPlainPayload(state);
}

std::string MQTTSensorController::_toPlainPayload(float value, int precision) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

std::string MQTTSensorController::_toPlainPayload(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string MQTTSensorController::_toPlainPayload(bool state) {
    std::stringstream ss;
    ss << (state ? "true" : "false");
    return ss.str();
}

std::string MQTTSensorController::_toJsonPayload(float value, int precision) {
    std::stringstream ss;
    ss << "{ \"value\": "<< std::fixed << std::setprecision(precision) << value << ")";
    return ss.str();
}

std::string MQTTSensorController::_toJsonPayload(int value) {
    std::stringstream ss;
    ss << "{ \"value\": " << value << "}";
    return ss.str();
}

std::string MQTTSensorController::_toJsonPayload(bool state) {
    std::stringstream ss;
    ss << "{ \"state\": " << (state ? "true" : "false") << "}";
    return ss.str();
}

std::string MQTTSensorController::_terminate(const std::string& payload) {
    std::stringstream ss;
    ss << payload << std::endl;
    return ss.str();
}
