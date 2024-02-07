#pragma once

class OccupancySensor {
public:
/*
    /// @brief Check whether occupancy detection is available.
    /// @return Return true if available. Oherwise false. 
    virtual bool isOccupancyAvailable() = 0;
*/
    /// @brief Read occupancy.
    /// @return Return true if room is occupied. Otherwise false.
    virtual bool readOccupancy() = 0;
};
