#pragma once

class HumidityObserver
{
public:

    /// @brief Inform of a change in humidity.
    /// @param humidity The humidity as a percentage. 
    virtual void onHumidity(float humidity) = 0;
};
