#pragma once

#include <string>
#include <vector>

#include "Chronos/TimeSpan.h"
#include "SmartSensorController.h"
#include "SmartSensorObserver.h"

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
    );

    void setTopicPrefix(const Topic& topicPrefix);

    bool isConnected();

    // Observer callbacks triggering outgoing messages.

    void onSwitchOnOff(bool on) override;
    void onFanSpeed(int speed) override;
    void onCabinetInspectionLightOnOff(bool on) override;
    void onCabinetBrightness(uint8_t brightness) override;
    void onCabinetColour(uint8_t hue, uint8_t sat) override;

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

    void onHeapUsage(uint32_t totalHeap, uint32_t freeHeap) override;

private:

    void _initInputs() override;
    void _serviceInputs() override;

    void _publishEntity(const char* entityName, bool state);
    void _publishEntity(const char* entityName, int value);
    void _publishEntity(const char* entityName, const std::string& payload);

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

    bool _handlePowerOnOffMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload);
    bool _handleSetCabinetInspectionLightMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload);
    bool _handleSetFanSpeedMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload);
    bool _handleSetFanModeMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload);
    bool _handleDisplayModeMessage(const _Topic& topic, const WildcardMatches& wildcardMatches, const std::string& payload);

    std::string _toPayload(float value, int precision = 1);
    std::string _toPayload(int value);
    std::string _toPayload(bool state);

    std::string _toPlainPayload(float value, int precision = 1);
    std::string _toPlainPayload(int value);
    std::string _toPlainPayload(bool state);
    
    std::string _toJsonPayload(float value, int precision = 1);
    std::string _toJsonPayload(int value);
    std::string _toJsonPayload(bool state);
    
    std::string _terminate(const std::string& payload);

    static std::vector<std::string> _split(std::string argsStr, char delimiter);

    // UserSettings& _userSettings;

    Topic _sensorTopicPrefix;
    MQTTConnection& _mqtt;
    Format _format;
    bool _retainState;
};
