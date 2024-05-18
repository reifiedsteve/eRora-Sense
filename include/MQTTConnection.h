#pragma once

#include <inttypes.h>
#include <map>
#include <functional>
#include <WiFi.h>
#include <PubSubClient.h>

#include "MQTTTopic.h"

class MQTTConnection
{
public:

    typedef MQTTTopic Topic;
    typedef std::vector<std::string> WildcardMatches;

    /// @brief Callback for when a connection is made with the broker.
    typedef std::function<void(const bool connected)> ConnectionCallback;

    /// @brief Call for when a message is received. Provides:
    /// the topic, any wildcard matches, and the message body.
    typedef std::function<bool(const Topic&, const WildcardMatches&, const std::string&)> MessageCallback;

    MQTTConnection(WiFiClient& client, const std::string& mqttServerHostName, int mqttPort = 1883);
    MQTTConnection(WiFiClient& client, const std::string& mqttServerHostName, int mqttPort, const std::string& username, const std::string& password);

    MQTTConnection(const MQTTConnection& rhs) = delete;
    MQTTConnection& operator=(const MQTTConnection& rhs) = delete;
    
    void setConnectionCallback(ConnectionCallback callbackConnection);

    void setup();

    inline bool isConnected() __attribute__((always_inline)){
        return _mqtt.connected();
    }

    void setTopicPrefix(const Topic& prefix);

    const Topic& getTopicPrefix() const;

    void subscribe(const Topic& relTopic, MessageCallback handler);
    bool unsubscribe(const Topic& relTopic);

    void publish(const Topic& relTopic, const std::string& message, bool retained = true);
    void publish(const Topic& relTopic, const String& message, bool retained = false);
    void publish(const Topic& relTopic, bool state, bool retained = false);
    void publish(const Topic& relTopic, long value, bool retained = false);
    void publish(const Topic& relTopic, double value, bool retained = false);

/*
    bool handlePacket(const Topic& topic, const uint8_t* payloadBytes, unsigned payloadCount);

    void send(const Topic& relTopic, uint8_t* payloadBytes, unsigned payloadCount);
*/

    void loop();

private:

    typedef std::map<Topic, MessageCallback> _Handlers;
    typedef _Handlers::const_iterator _HandlersConstIter;
    typedef _Handlers::iterator _HandlersIter;

    bool _isConnected();

    void _setTopicPrefix(const Topic& prefix);

    void _publish(const Topic& relTopic, const String& message, bool retained);
    void _publish(const Topic& relTopic, const char* message, bool retained);

    void _subscribeAll();
    void _subscribe(const Topic& topic);

    void _unsubscribeAll();
    void _unsubscribe(const Topic& topic);

    void _onMqttRawMessage(char* topic, byte* payload, unsigned int length);
    bool _onMessage(const Topic& topic, const std::string message);

    std::string _generateClientID();

    static bool _startsWith(const std::string& str, const std::string& prefix);

    WiFiClient& _wiFiClient;
    PubSubClient _mqtt;

    std::string _mqttServerHostName;
    int _mqttPort;

    bool _connected;

    std::string _username, _password;

    Topic _prefix;

    ConnectionCallback _connectionCallback;
    _Handlers _handlers;
};