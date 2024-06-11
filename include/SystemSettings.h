#pragma once

#include "Settings.h"

class SystemSettings : public Settings
{
public:

    static const int  DefaultLightstripNumLEDs;
    static const int  DefaultPSUVoltage;
    static const int  DefaultPSUMilliamps;
    static const bool DefaultApplyGamma;

    SystemSettings()
      : Settings("sys")
    {}

    inline void setDeviceName(const std::string& name) {  // TODO: per device? Or per v-lamp?
        _putSetting("dev-name", name);
    }

    inline std::string getDeviceName() const {
        return _getSetting("dev-name", _getDefaultDeviceName());
    }

    inline void setDeviceDescriptiveName(const std::string& name) {  // TODO: per device? Or per v-lamp?
        _putSetting("dev-desc", name);
    }

    inline std::string getDeviceDescriptiveName() const {
        return _getSetting("dev-desc", "3D Printer Cabinet");
    }

    inline void useManualNetworkConfig(bool useStatic) {
        _putSetting("ip-static", useStatic);
    }

    inline bool useManualNetworkConfig() const {
        return _getSetting("ip-static", false);
    }

    inline void setStaticIP(const IPAddress& address) {
        _putStringLegacy("static-ip", address.toString());
    }

    inline IPAddress getStaticIP() const {
        IPAddress addr;
        addr.fromString(_getStringLegacy("static-ip", "0.0.0.0"));
        return addr;
    }

    inline void setSubnet(const IPAddress& address) {
        _putStringLegacy("subnet", address.toString());
    }

    inline IPAddress getSubnet() const {
        IPAddress addr;
        addr.fromString(_getStringLegacy("subnet", "0.0.0.0"));
        return addr;
    }

    inline void setGateway(const IPAddress& address) {
        _putStringLegacy("gateway", address.toString());
    }

    inline IPAddress getGateway() const {
        IPAddress addr;
        addr.fromString(_getStringLegacy("gateway", "0.0.0.0"));
        return addr;
    }

    inline void setDNS1(const IPAddress& address) {
        _putStringLegacy("dns1", address.toString());
    }

    inline IPAddress getDNS1() const {
        IPAddress addr;
        addr.fromString(_getStringLegacy("dns1", "0.0.0.0"));
        return addr;
    }

    inline void setDNS2(const IPAddress& address) {
        _putStringLegacy("dns2", address.toString());
    }

    inline IPAddress getDNS2() const {
        IPAddress addr;
        addr.fromString(_getStringLegacy("dns2", "0.0.0.0"));
        return addr;
    }

    inline void setWifiSSID(const std::string& ssid) {
        _putSetting("wifi-ssid", ssid);
    }

    inline std::string getWifiSSID() const {
        return _getSetting("wifi-ssid", "133381B");    //  TODO: remove this specific default value in final release.
    }

    inline void setWifiPSK(const std::string& password) {
        _putSetting("wifi-psk", password);
    }

    inline std::string getWifiPSK() const {
        // return PersistentSettings::get("wifi", "psk", ""); // TODO: use this.
        return _getSetting("wifi-psk", "spongebob2000");    //  TODO: remove this specific default value in final release.
    }

    #if 0

    inline void setNTPDomain(const std::string& domain) {
        _putSetting("ntp-domain", domain);
    }

    inline std::string getNTPDomain() const {
        return _getSetting("ntp-domain", "pool.ntp.org");
    }

    inline void setTimezone(const std::string& tz) {
        _putSetting("tz", tz);
    }

    inline std::string getTimezone() const {
        return _getSetting("tz", "GMT0");
    }

    inline void setTimezoneName(const std::string& tzName) {
        _putSetting("tz-name", tzName);
    }

    inline std::string getTimezoneName() const {
        return _getSetting("tz-name", "GMT");
    }

    #endif
    
    inline void setMQTTBrokerAddress(const std::string& address) {
        _putSetting("mqtt-ip", address);
    }

    inline std::string getMQTTBrokerAddress() const {
        return _getSetting("mqtt-ip", "mqtt.local");
    }

    inline void setMQTTBrokerPort(unsigned portNo) {
        _putSetting("mqtt-port", (int)portNo);
    }

    inline unsigned getMQTTBrokerPort() const {
        return (unsigned)_getSetting("mqtt-port", 1883);
    }

    inline void setMQTTBrokerUsername(const std::string& address) {
        _putSetting("mqtt-user", address);
    }

    inline std::string getMQTTBrokerUsername() const {
        return _getSetting("mqtt-user", "");
    }

    inline void setMQTTBrokerPassword(const std::string& address) {
        _putSetting("mqtt-psk", address);
    }

    inline std::string getMQTTBrokerPassword() const {
        return _getSetting("mqtt-psk", "");
    }

    inline void setMQTTTopicPrefix(const std::string& prefix) {
        _putSetting("mqtt-prefix", prefix);
    }

    inline std::string getMQTTTopicPrefix() const {
        return _getSetting("mqtt-prefix", "eRora/sensors/office");
    }

    #if 0

    inline void setNumberOfLEDs(unsigned count) {
        _putSetting("led-count", (int)count);
    }

    inline unsigned getNumberOfLEDs() const {
        return _getSetting("led-count", DefaultLightstripNumLEDs);
    }

    inline void setGammaCorrect(bool correct) {
        _putSetting("gamma-fix", correct);
    }

    inline bool getGammaCorrect() const {
        return _getSetting("gamma-fix", DefaultApplyGamma);
    }

    inline void setLEDsPSUVoltage(unsigned volts) {
        _putSetting("psu-volts", (int)volts);
    }

    inline unsigned getLEDsPSUVoltage() const {
        return _getSetting("psu-volts", DefaultPSUVoltage);
    }

    inline void setLEDsPSUMilliamps(unsigned milliamps) {
        _putSetting("psu-mA", (int)milliamps);
    }

    inline unsigned getLEDsPSUMilliamps() const {
        return _getSetting("psu-mA", DefaultPSUMilliamps);
    }

    inline void setManagePower(bool limitPower) {
        _putSetting("psu-limit", limitPower);
    }

    inline bool getManagePower() const {
        return _getSetting("psu-limit", true);
    }

    #endif

private:

    static std::string _getDefaultDeviceName();
};

