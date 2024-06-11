#include <sstream>

#include "MQTTConnection.h"
#include "Diagnostics/Logging.h"

MQTTConnection::MQTTConnection(WiFiClient& client, const std::string& mqttServerHostName, int mqttPort)
  : _wiFiClient(client)
  , _mqtt(_wiFiClient)
  , _mqttServerHostName(mqttServerHostName)
  , _mqttPort(mqttPort)
  , _connected(false)
  , _username()
  , _password()
  , _prefix()
  , _connectionCallback()
  , _handlers()
{}

MQTTConnection::MQTTConnection(WiFiClient& client, const std::string& mqttServerHostName, int mqttPort, const std::string& username, const std::string& password)
  : _wiFiClient(client)
  , _mqtt(_wiFiClient)
  , _mqttServerHostName(mqttServerHostName)
  , _mqttPort(mqttPort)
  , _username(username)
  , _password(password)
  , _prefix()
  , _connectionCallback()
  , _handlers()
{}

void MQTTConnection::setup() {
    _mqtt.setServer(_mqttServerHostName.c_str(), _mqttPort);
    _mqtt.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->_onMqttRawMessage(topic, payload, length);
    });
    Log.noticeln("MQTT: Using MQTT broker on %s on port %d", _mqttServerHostName.c_str(), _mqttPort);
    _subscribeAll();
    Log.noticeln("MQTT: Not yet connected. Number of pre-registered handlers is %d", _handlers.size());
}
    
void MQTTConnection::setTopicPrefix(const Topic& prefix) {
    _setTopicPrefix(prefix);
}

const MQTTConnection::Topic& MQTTConnection::getTopicPrefix() const {
    return _prefix;
}

void MQTTConnection::subscribe(const Topic& relTopic, MessageCallback handler) {
    //Log.verboseln("MQTT: subscribing to topic %s", _prefix + relTopic);
    // TODO: check that we don't already have a handler for this topic.
    //if (_handlers.erase(relTopic) > 0) 
    //    Log.warningln("MQTT: changing subscription for topic %s", _prefix + relTopic);
    _handlers[relTopic] = handler;
    _subscribe(_prefix + relTopic);
}

bool MQTTConnection::unsubscribe(const Topic& relTopic) {
    _HandlersIter iter = _handlers.find(relTopic);
    bool found(iter != _handlers.end());
    if (found) {
        _unsubscribe(_prefix + relTopic);
        _handlers.erase(iter);
    }
    return found;
}

void MQTTConnection::publish(const Topic& relTopic, const String& message, bool retained) {
    _publish(relTopic, message, retained);
}

void MQTTConnection::publish(const Topic& relTopic, const std::string& message, bool retained) {
    _publish(relTopic, String(message.c_str()), retained); // TODO: horrible that we have *two* string types!
}

void MQTTConnection::publish(const Topic& relTopic, bool state, bool retained) {
    std::ostringstream os;
    os << (state ? "true" : "false");
    _publish(relTopic, os.str().c_str(), retained);
}

void MQTTConnection::publish(const Topic& relTopic, long value, bool retained) {
    std::ostringstream os;
    os << value;
    _publish(relTopic, os.str().c_str(), retained);
}

void MQTTConnection::publish(const Topic& relTopic, double value, bool retained) {
    std::ostringstream os;
    os << value;
    _publish(relTopic, os.str().c_str(), retained);
}

/*
void MQTTExchange::send(const std::string& relTopic, uint8_t* payloadBytes, unsigned payloadCount) {
    _mqtt.publish((_prefix + relTopic).c_str(), payloadBytes, payloadCount);
}
*/

void MQTTConnection::loop()
{
    // TODO: exec this as its own task to avoid blocking
    // for 5 secs when the broker is down,

    bool connected(_isConnected());

    if (!_connected && connected) {
        if (_connectionCallback) {
            _connectionCallback(true);
        }
    }
    
    else if (_connected && !connected) {
        if (_connectionCallback) {
            _connectionCallback(false);
        }
    }

    _mqtt.loop();
}

bool MQTTConnection::_isConnected()
{
    static unsigned long attemptInterval(30000);
    static unsigned long lastAttemptTime(0);

    bool connected(_mqtt.connected());

    if (!connected)
    {
        unsigned long now(millis());

        if (now - lastAttemptTime > attemptInterval)
        {
            std::string clientId = _generateClientID();

            bool connected(false);

            if ((_username != "") || (_password != "")) {
                Log.infoln("MQTT: attempting connection (using credentials) as client \"%s\".", clientId.c_str());
                connected = _mqtt.connect(clientId.c_str(), _username.c_str(), _password.c_str());
                Log.infoln("MQTT: attempted - now %sconnected as client \"%s\".", (connected ? "" : "not "), clientId.c_str());
            } else {
                Log.infoln("MQTT: attempting connection (anonymously) as client \"%s\".", clientId.c_str());
                connected = _mqtt.connect(clientId.c_str());
                Log.infoln("MQTT: attempted - now %sconnected as client \"%s\".", (connected ? "" : "not "), clientId.c_str());
            }

            if (connected) {
                Log.infoln("MQTT: Connected");
                // Once connected, publish an announcement...
                // _mqtt.publish((std::string("neosmartlight/") + _neoLightsName + "/announce").c_str(), "hello world");
                _subscribeAll();
                Log.infoln("MQTT: Subscribed to %d topics.", _handlers.size());
                // connected = true;
            }
            else {
                Log.warningln("MQTT: Failed to connect to MQTT broker at %s on %d (returned %d).", _mqttServerHostName.c_str(), _mqttPort, _mqtt.state());
            }

            lastAttemptTime = now;
        }
    }

    return connected;
}

void MQTTConnection::_setTopicPrefix(const Topic& prefix)
{
    Log.noticeln("MQTT: setting topic prefix to \"%s\"", prefix.str().c_str());

    if (prefix != _prefix) {
        _unsubscribeAll();
        _prefix = prefix;
        _subscribeAll();
    }
}

void MQTTConnection::_publish(const Topic& relTopic, const String& message, bool retained) {
    _publish(relTopic, message.c_str(), retained);
}

void MQTTConnection::_publish(const Topic& relTopic, const char* message, bool retained) {
    const std::string topic((_prefix + relTopic).str());
    // Log.verboseln("MQTT: publish %s %s", topic.c_str(), message);
    _mqtt.publish(topic.c_str(), message, retained);
}

void MQTTConnection::_subscribeAll() {
    for (_HandlersConstIter iter = _handlers.begin(); iter != _handlers.end(); ++iter) {
        const Topic& relTopic = iter->first;
        _subscribe(_prefix + relTopic);
    }
}

void MQTTConnection::_subscribe(const Topic& topic) {
    _mqtt.subscribe(topic.str().c_str());
    Log.noticeln("MQTT: sent subscription to topic %s", topic.str().c_str());
}

void MQTTConnection::_unsubscribeAll() {
    for (_HandlersConstIter iter = _handlers.begin(); iter != _handlers.end(); ++iter) {
        const Topic& relTopic = iter->first;
        _unsubscribe(_prefix + relTopic);
    }
}

void MQTTConnection::_unsubscribe(const Topic& topic) {
    _mqtt.unsubscribe(topic.str().c_str());
    Log.noticeln("MQTT: sent unsubscription from topic %s", topic.str().c_str());
}

void MQTTConnection::_onMqttRawMessage(char* topic, byte* payload, unsigned int length) {
    std::string message((char*)payload, length);
    _onMessage(Topic(topic), message);
}

bool MQTTConnection::_onMessage(const Topic& incomingTopic, const std::string payload)
{
    Log.noticeln("MQTT: incoming topic \"%s\" with payload \"%s\"", incomingTopic.str().c_str(), payload.c_str());

    bool gotHandler(false);
    bool handled(false);

    for (_HandlersConstIter iter = _handlers.begin(); iter != _handlers.end(); ++iter)
    {
        const Topic& relTopic(iter->first);

        Topic subscribedTopic(_prefix + relTopic);
        WildcardMatches wildcardMatches;

        if (subscribedTopic.matches(incomingTopic, std::inserter(wildcardMatches, wildcardMatches.begin())))
        {
            MessageCallback handler(iter->second);
            gotHandler = true;

            if (handler(incomingTopic, wildcardMatches, payload)) {
                handled = true;
            }

            else {
                Log.errorln("MQTT: handler for MQTT topic \"%s\" rejected the message (possibly ill-formed payload was \"%s\").", incomingTopic.str().c_str(), payload.c_str());
            }
        }
    }

    if (!gotHandler) {
        Log.errorln("MQTT: incoming MQTT topic \"%s\" had no corresponding handler.", incomingTopic.str().c_str());
    }

    return handled;
}

std::string MQTTConnection::_generateClientID()
{
    // TODO: Is this the best generation method?

    std::ostringstream os;
    os << "eRora";
    for (int i = 0; i < 4; ++i) {
        int digit(random(10));
        os << (digit + '0');
    }
    return os.str();
}

bool MQTTConnection::_startsWith(const std::string& str, const std::string& prefix) {
    return (str.length() >= prefix.length()) && (str.rfind(prefix, 0) == 0); // TODO: rfind?
}
