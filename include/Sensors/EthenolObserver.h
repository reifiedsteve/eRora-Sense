#pragma once

#include <stdint.h>

class EthenolObserver
{
public:

    /// @brief Inform of a change in ethenol levels.
    /// @param pressure The hydrogen level in ppm. 
    virtual void onEthenol(uint16_t ethenol) = 0;
};
