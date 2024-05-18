#pragma once

class OccupancySensor {
public:

/*
    /// @brief Is the occupancy sensor in a steady-state and able to provide
    /// measurements?
    /// @return Returns true if in the steady-state; otherwise false;
    virtual bool isOccupancySensorReady() = 0;
*/
    /// @brief Check whether occupancy detection is available.
    /// @return Return true if available. Otherwise false. 
    virtual bool isOccupancyAvailable() = 0;

    /// @brief Read occupancy.
    /// @return Return true if room is occupied. Otherwise false.
    virtual bool readOccupancy() = 0;
};
