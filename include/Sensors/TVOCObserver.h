#pragma once

#include <stdint.h>

class TVOCObserver
{
public:

    /// @brief Inform of a change in TVOC level (in parts per billion).
    /// @param present Returns total TVOC in ppb,
    virtual void onTVOC(uint16_t tvoc) = 0;
};
