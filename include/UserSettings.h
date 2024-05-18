#pragma once

#include <iomanip>

#include "Settings.h"

// #define PERSIST_CURRENT_STATE

class UserSettings : public Settings
{
public:

    // Note: we *avoid* peristing any "current" state and only
    // persist things explicitly saved by the user, as we wish
    // to void writing too often to the flash drive (as that
    // could cause premature failure of the drive).
    // i.e. it would be beneficial to e.g. persist the current on/off,
    // brightness etc, but that would mean saving them *every* time
    // they changed (including via any smart home logic), which
    // could be quite frequent (and more frequently than we
    // should probably write to the flash as it has limited write
    // lifespan).

    UserSettings(const char* settingsNamespace)
      : Settings(settingsNamespace)
    {}

    // Are there any user settings?

};
