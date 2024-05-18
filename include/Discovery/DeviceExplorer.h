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

#include <Stream.h> // Needed by AsyncUDP.h (which for some sloppy reason does not include it itself!!!).
#include <AsyncUDP.h>

#include <mutex>
#include <list>
#include <cmath>
#include <mutex>

#include "Chronos/CountdownTimer.h"

#include "Discovery/DeviceInformation.h"
/*
#include "Discovery/InquiryMessage.h"
#include "Discovery/ResponseMessage.h"
*/
#include "Messages/DeviceDiscoveryMessage.h"

/// @brief Provides a means to search for other eRora devices on the LAN.
class DeviceExplorer
{
public:

    /// @brief Change event type for reporting whether a device has just
    /// been discovered or is now deemed to have gone from the LAN.
    enum class DeviceEvent : uint8_t
    {
        /// @brief No event occurred.
        None = 0x00,

        /// @brief An eRora device has been discovered.
        ComeOnline = 0x01,

        /// @brief An eRora device is deemed to have gone from the LAN.
        GoneOffline = 0x02,

        /// @brief An online device changed name.
        NameChange = 0x03
    };

    /// @brief Ticks of time (milliseconds).
    typedef CountdownTimer::Ticks Ticks;

    /// @brief Maximum length of a device name. 
    static const int MaxDeviceNameLen = 32;
    
    /// @brief A callback for when an observer wishes to be informed of eRora 
    /// devices coming and going from the LAN.
    /// @param deviceName The name of this device. Note: will be truncated to 32 characters if longer.
    /// @param address The IP address of the remote device.
    /// @param event Indicates whether the device appeared or disappeared from view.
    typedef std::function<void (const DeviceInformation& deviceDetails, DeviceEvent status)> DeviceDiscoveryCallback;

    /// @brief Constructor where we must provide details of *this* eRora device.
    /// @param deviceType The device type.
    /// @param deviceName The devive name.
    /// @param noOfLEDs Device type specific info - for this device type, the number of LEDs.
    /// @param broadcastInterval The broadcast interval for searching out for other devices.
    /// @param cleanUpInterval The timeout beyond which we assume that a remote device has gone offline.
    explicit DeviceExplorer(DeviceCategory deviceType, const std::string& deviceName, uint16_t noOfLEDs, Ticks broadcastInterval, Ticks cleanUpInterval);

    DeviceExplorer(const DeviceExplorer&) = delete;

    /// @brief Register an observer to be informed of any devices coming or going.
    /// @param onDiscoveryFunc The callback to be called when a device event occurs.
    /// @param provideInitialList Whether or not to provide the current list of devices
    /// initially upon registration (i.e. immediately, via the provided callback).
    void registerForChanges(DeviceDiscoveryCallback onDiscoveryFunc, bool provideInitialList = true);

    /// @brief Runtime initialisation.
    void setup();

    /// @brief The iterative main loop which must be called periodically.
    void loop();

private:

    struct _WitnessedDevice
    {
        inline _WitnessedDevice(const DeviceInformation& details, Ticks elapsed)
          : deviceDetails(details)
          , lastSeenTime(elapsed)
        {}

        inline bool operator==(const _WitnessedDevice& rhs) const {
            // Comparing just the IP is sufficient for uniqueness.
            return (deviceDetails.address() == rhs.deviceDetails.address());
        }

        DeviceInformation deviceDetails;
        Ticks lastSeenTime; 
    };

    typedef std::list<_WitnessedDevice> _WitnessedDevices;
    typedef _WitnessedDevices::iterator _WitnessedDevicesIter;
    typedef _WitnessedDevices::const_iterator _WitnessedDevicesConstIter;
    
    typedef std::list<DeviceDiscoveryCallback> _Observers;
    typedef _Observers::const_iterator _ObserversOonstIter;
    typedef _Observers::iterator _ObserversIter;
    
    typedef std::mutex _Mutex;
    typedef std::lock_guard<_Mutex> _ScopedLock;
    
    // static const _InquiryMessage _inquiryMessage;

    void _onPacket(AsyncUDPPacket& packet);

    void _handleMessage(const IPAddress& originatingAddress, BinaryMessage* message);
    void _handleDiscoveryMessage(const DeviceInformation& deviceDetails);

    /* 
    void _handleInquiry(AsyncUDPPacket& packet);
    void _handleResponse(AsyncUDPPacket& packet);
    */

    DeviceEvent _processDeviceMessage(const DeviceInformation& deviceDetails);

    void _removeOfflineDevices(Ticks expiryTime);
    void _removeOfflineDevices(Ticks expiryTime, _WitnessedDevices& removed);

    void _findStaleDevices(Ticks expiryTime, _WitnessedDevices& staleDevices);
    void _removeDevices(_WitnessedDevices& devicesToRemove);

    _WitnessedDevicesIter _findDevice(const IPAddress& deviceAddress);

    void _notifyOfOfflineDevices(const _WitnessedDevices& offlineDevices);
    void _notifyOfOnlineDevice(const DeviceInformation& deviceDetails); 
    void _notifyOfModifyDevice(const DeviceInformation& deviceDetails);
    void _notifyOfOfflineDevice(const DeviceInformation& deviceDetails);
    void _notifyOfDeviceState(const DeviceInformation& deviceDetails, DeviceEvent state);

    DeviceCategory _deviceType;
    const std::string _deviceName;
    uint16_t _noOfLEDs;

    DeviceDiscoveryMessage _thisDeviceMessage;

    Stopwatch _elapsedTimer;

    CountdownTimer _broadcastCountdown;
    CountdownTimer _cleanUpCountdown;

    uint16_t _portNo;
    AsyncUDP _udp;

    _Observers _observers;
    
    _WitnessedDevices _knownOnlineDevices;
    _Mutex _mutex;
};

