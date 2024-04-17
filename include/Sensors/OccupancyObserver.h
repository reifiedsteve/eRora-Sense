#pragma once

class OccupancyObserver
{
public:

    /// @brief Inform of a change in occupancy.
    /// @param present Returns true if occupant(s) are present. Otherwise false. 
    virtual void onOccupancy(bool present) = 0;
};
