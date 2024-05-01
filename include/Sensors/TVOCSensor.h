#pragma once

#include <stdint.h>

class TVOCSensor
{
public:

    /// @brief Read the TVOC in ppm.
    /// @return The TVOC in ppm.
    virtual uint16_t readTVOC() = 0;
};