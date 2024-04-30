#pragma once

#include <stdint.h>

class CO2Observer
{
public:

    /// @brief Inform of a change in CO2 level (in total parts-per-million).
    /// @param present Returns CO2 in ppm. 
    virtual void onCO2(uint16_t co2) = 0;
};
