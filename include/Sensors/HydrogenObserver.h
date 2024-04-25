#pragma once

class HydrogenObserver
{
public:

    /// @brief Inform of a change in hydrogen levels.
    /// @param pressure The hydrogen level in ppm. 
    virtual void onHydrogen(uint16_t hydrogen) = 0;
};
