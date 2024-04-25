#pragma once

class HydrogenSensor {
public:
/*
    /// @brief Check whether a gas level measurement is available.
    /// @return Return true if available. Oherwise false. 
    virtual bool isGasLevelAvailable() = 0;
*/
    /// @brief Read hydrogen level in ppm.
    /// @return Hydrogen level as ppm.
    virtual uint16_t readHydrogen() = 0;
};
