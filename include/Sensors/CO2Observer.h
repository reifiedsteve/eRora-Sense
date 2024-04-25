#pragma once

#include <stdint.h>

class CO2Observer
{
public:

    /// @brief Inform of a change in CO2 level.
    /// @param present Returns CO2 in ug/m3. 
    virtual void onCO2(uint16_t co2) = 0;
};
