#pragma once

class GasLevelSensor {
public:
/*
    /// @brief Check whether a gas level measurement is available.
    /// @return Return true if available. Oherwise false. 
    virtual bool isGasLevelAvailable() = 0;
*/
    /// @brief Read gas level.
    /// @return Gas level in units of KOhms.
    virtual float readGasLevel() = 0;
};
