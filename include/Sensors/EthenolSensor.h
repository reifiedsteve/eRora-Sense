#pragma once

#include <stdint.h>

class EthenolSensor {
public:
/*
    /// @brief Check whether a gas level measurement is available.
    /// @return Return true if available. Oherwise false. 
    virtual bool isGasLevelAvailable() = 0;
*/
    /// @brief Read ethenol level in ppm.
    /// @return Ethenol level as ppm.
    virtual uint16_t readEthenol() = 0;
};
