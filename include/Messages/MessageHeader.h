#pragma once

#if false

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

#include <inttypes.h>
#include "Messages/MessageType.h"

struct MessageHeader
{
    inline MessageHeader(MessageType type) 
        : preamble{'e', 'R', 'o', 'r', 'a' }
        , messageType((uint8_t)type)
    {}

    inline bool isValid() const {
        return (preamble[0] == 'e') && (preamble[1] == 'R') && (preamble[2] == 'o') && (preamble[3] == 'r') && (preamble[4] == 'a');
    }

    inline MessageType getMessageType() const {
        return (MessageType)messageType;
    }

    // !!!!!!!!!!!!!!!! --- IMPORTANT --- !!!!!!!!!!!!!!!!
    // This class is defined strictly by these member data,
    // their exact size, their relative ordering, and how
    // they are packed. They represent the exact structure
    // of the bytes passed over the network, adhering to the
    // discovery network protocol. Do not change them. Do
    // not add any new member data. Do not remove any.

    const uint8_t preamble[5];
    uint8_t messageType;  // This is uint8_t to ensure its fixed size (byte).
    
} __attribute__((packed));

#endif
