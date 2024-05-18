#pragma once

class AirPressureObserver
{
public:

    /// @brief Inform of a change in air pressure.
    /// @param pressure The air pressure in hPa. 
    virtual void onAirPressure(float hPa) = 0;
};
