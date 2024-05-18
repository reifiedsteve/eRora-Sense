#pragma once

#include <inttypes.h>

#include "Messages/MessageType.h"

struct BinaryMessage
{
    inline BinaryMessage(MessageType type) 
        : preamble{'e', 'R', 'o', 'r', 'a' }
        , messageType((uint8_t)type)
    {}

    inline bool headerIsValid() const {
        return (preamble[0] == 'e') && (preamble[1] == 'R') && (preamble[2] == 'o') && (preamble[3] == 'r') && (preamble[4] == 'a');
    }

    inline MessageType getMessageType() const {
        return (MessageType)messageType;
    }

    const uint8_t preamble[5];
    uint8_t messageType;    

} __attribute__((packed));

