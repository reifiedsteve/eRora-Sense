#pragma once

// Disabling logging typically saves about 1.0% of progmem.
// #define DISABLE_LOGGING

#ifdef DISABLE_LOGGING
#include <iostream>
#endif

#include <ArduinoLog.h>

void initLogging(int logLevel);

