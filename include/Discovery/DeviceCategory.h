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

// Device catagories:
// 0x0X, 0x1X - lights
// 0x2X, 0x3X - blinds/shades/curtains
// 0x4X, 0x4x - IR devices

/// @brief The type of smart device.
enum class DeviceCategory : uint8_t
{
    /// @brief Not an embedded device type, but used to 
    /// identify (e.g.) a management device.
    Reserved = 0x00, 

    /// @brief An LED light strip (e.g. ws2812B)
    Lightstrip = 0x01,

    /// @brief A single bulbed light that can change colour.
    DeskLamp = 0x02,

    /// @brief A 2D LED matrix light.   
    Matrix = 0x03,

    /// @brief A powered roller blind.
    RollerBlind = 0x21,

    /// @brief A network connected universal IR remote.
    UniversalRemote = 0x41,

    /// @brief A temperature sensor.
    TemperatureSensor = 0x60,

    /// @brief A humidity sensor.
    HumiditySensor = 0x61,

    /// @brief A temperature/humidity sensor.
    TemperaturHumiditySensor = 0x62,

    /// @brief An occupancy sensor.
    OccupancySensor = 0x63,

    /// @brief A multi-sensor.
    MultiSensor = 0x6f
};