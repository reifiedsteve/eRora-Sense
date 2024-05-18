#include <sstream>

#include "SystemSettings.h"
#include "MACAddress.h"

std::string SystemSettings::_getDefaultDeviceName() {
    std::stringstream ss;
    ss << "eRora-" << MACAddress::local().str().substr(6); /*  << ".local"   implicit anyway? */
    return ss.str();
}

