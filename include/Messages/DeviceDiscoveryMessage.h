#pragma once

#include <string>
#include <algorithm>

#include "Discovery/DeviceCategory.h"
#include "Messages/BinaryMessage.h"

struct DeviceDiscoveryMessage : public BinaryMessage
{
    DeviceDiscoveryMessage(DeviceCategory type, const std::string& deviceName) 
      : BinaryMessage(MessageType::DeviceDiscovery)
      , deviceType((uint8_t)type)
      , nameLen((uint8_t)std::min(deviceName.size(), (size_t)32))
      , nameBytes()
    {
        std::fill_n(&nameBytes[nameLen], 32-nameLen, 0);
        std::copy_n(&deviceName[0], nameLen, &nameBytes[0]);
    }

    std::string getDeviceName() const {
        return std::string((char*)nameBytes, (size_t)nameLen);
    }

    DeviceCategory getDeviceType() {
        return (DeviceCategory)deviceType;
    }

    uint8_t deviceType;
    uint8_t nameLen;
    uint8_t nameBytes[32]; // max 32. Pad full 32 with zeros.

} __attribute__((packed));

