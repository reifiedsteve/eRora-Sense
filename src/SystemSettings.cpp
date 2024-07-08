#include <sstream>

#include "SystemSettings.h"
#include "MACAddress.h"

const int SystemSettings::DefaultLightstripNumLEDs = 30;
const int  SystemSettings::DefaultPSUVoltage = 5;
const int  SystemSettings::DefaultPSUMilliamps = 1200;
const bool SystemSettings::DefaultApplyGamma = true;

std::string SystemSettings::_getDefaultDeviceName() {
    std::stringstream ss;
    ss << "eRora-" << MACAddress::local().str().substr(6); /*  << ".local"   implicit anyway? */
    return ss.str();
}

