#pragma once

#include <stdint.h>

class TVOCSensor
{
public:

/*
    /// @brief Is the TVOC sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    virtual bool isTVOCSensorReady() = 0;
*/
    /// @brief Check whether a TVOC measurement is available.
    /// @return Return true if available. Otherwise false. 
    virtual bool isTVOCAvailable() = 0;

    /// @brief Read the TVOC in parts-per-million.
    /// @return The TVOC in ppm.
    virtual float readTVOC() = 0;
};