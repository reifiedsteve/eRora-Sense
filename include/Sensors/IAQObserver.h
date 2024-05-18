#pragma once

class IAQObserver
{
public:

    /// @brief Inform when IAQ is available (or not).
    /// @param available Returns true if available; otherwise false.
    virtual void onIAQAvailability(bool available) = 0;

    /// @brief Inform of a change in IAQ level.
    /// @param present IAQ level.. 
    virtual void onIAQ(float iaq) = 0;
};
