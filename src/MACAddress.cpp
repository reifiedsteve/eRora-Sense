#include "MACAddress.h"

#include <WiFi.h>
#include <iomanip>
#include <sstream>

MACAddress MACAddress::zero() {
    return MACAddress();
}

MACAddress MACAddress::local() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    return MACAddress(mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

std::string MACAddress::str(const char* delimiter) const {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0');
    ss << (int)_a << delimiter << (int)_b << delimiter << (int)_c << delimiter << (int)_d << delimiter << (int)_e << delimiter << (int)_f;
    return ss.str();
}
