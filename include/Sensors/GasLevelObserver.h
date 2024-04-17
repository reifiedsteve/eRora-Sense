#pragma once

class GasLevelObserver
{
public:

    /// @brief Inform of a change in gas levels.
    /// @param pressure The gas level in KOhms. 
    virtual void onGasLevel(float gasLevel) = 0;
};
