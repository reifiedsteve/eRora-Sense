#pragma once

#include <stdint.h>

class TVOCObserver
{
public:

    /// @brief Inform of a change in TVOC level.
    /// @param present Returns TVOC in ug/m3. 
    virtual void onTVOC(uint16_t tvo) = 0;
};
