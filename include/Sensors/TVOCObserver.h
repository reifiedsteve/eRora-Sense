#pragma once

#include <stdint.h>

class TVOCObserver
{
public:

    /// @brief Inform of a change in TVOC level (in parts per billion).
    /// @param present Returns total TVOC in ppm.
    virtual void onTVOC(float tvoc) = 0;
};
