#pragma once

// ####################################################################################
// 
//                              `_`     `_,_`  _'                                  `,
//                             -#@@- >O#@@@@u B@@>                                 8@E
//     :)ilc}` `=|}uccccVu}r"   VQz `@@#Mhzk= |8M   `=v}ucccccuY),    `~v}uVVcccccV#@$
//   ^Q@#EMqK.I#@QRdMqqMdRQ@@Q, Q@B `@@BqqqW^ W@@` e@@QRdMMMMbEQ@@8: i#@BOMqqqqqqqM#@$
//   D@@`    )@@x          <@@T Q@B `@@q      W@@`>@@l          :@@z`#@d           Q@$
//   D@#     ?@@@##########@@@} Q@B `@@q      W@@`^@@@##########@@@y`#@W           Q@$
//   0@#     )@@d!::::::::::::` Q@B `@@M      W@@`<@@E!::::::::::::``#@b          `B@$
//   D@#     `m@@#bGPP}         Q@B `@@q      W@@` 3@@BbPPPV         y@@QZPPPPPGME#@8=
//   *yx       .*icywwv         )yv  }y>      ~yT   .^icywyL          .*]uywwwwycL^-
// 
//      (c) 2022 Reified Ltd.     W: www.reified.co.uk     E: info@reified.co.uk
// 
// ####################################################################################

#include <IPAddress.h>
#include <string>
#include <sstream>
#include "DeviceCategory.h"

/// @brief Encapsulates what we know about a particular device.
class DeviceInformation
{
public:

    /// @brief Contruct some device information.
    /// @param address The device IP address.
    /// @param name The device name.
    /// @param category The category of device.
    DeviceInformation(IPAddress address, const std::string& name, DeviceCategory category)
        : _address(address)
        , _name(name)
        , _category(category)
    {}

    /// @brief Obtain the device IP address.
    /// @return The IP address.
    inline const IPAddress& address() const __attribute__((always_inline)) {
        return _address;
    }
    
    /// @brief Obtain the device name (i.e. descriptive text).
    /// @return The device name.
    inline const std::string& name() const __attribute__((always_inline)) {
        return _name;
    }
    
    /// @brief Obtain the device category.
    /// @return The device category.
    inline DeviceCategory category() const __attribute__((always_inline)) {
        return _category;
    }

    /// @brief Render the device iformation as a string.
    /// @return The textually rendered device information.
    std::string str() const {
        std::stringstream ss;
        ss << "{" << _address.toString().c_str() << ", " << (int)_category << ", \"" << _name << "\")";
        return ss.str();
    }

private:

    IPAddress _address;
    std::string _name;
    DeviceCategory _category;
};