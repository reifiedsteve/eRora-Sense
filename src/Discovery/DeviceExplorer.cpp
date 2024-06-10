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

#include "Discovery/DeviceExplorer.h"
#include "Diagnostics/Logging.h"

DeviceExplorer::DeviceExplorer(DeviceCategory deviceType, const std::string& deviceName, uint16_t noOfLEDs, Ticks broadcastInterval, Ticks cleanUpInterval)
  : _deviceType(deviceType)
  , _deviceName(deviceName)
  , _noOfLEDs(noOfLEDs)
  , _thisDeviceMessage(deviceType, _deviceName)
  , _elapsedTimer(Stopwatch::State::Running) // Note: must start this running before the other timers.
  , _broadcastCountdown(broadcastInterval, CountdownTimer::State::Stopped)
  , _cleanUpCountdown(cleanUpInterval, CountdownTimer::State::Running)
  , _portNo(12345)
  , _udp()
  , _observers()
  , _knownOnlineDevices()
  , _mutex()
{
    Log.infoln("DeviceExplorer: device name set as \"%s\".", _deviceName.c_str());
}

void DeviceExplorer::registerForChanges(DeviceDiscoveryCallback onDiscoveryFunc, bool provideInitialList)
{
    _observers.push_back(onDiscoveryFunc);

    if (provideInitialList) {
        for (auto deviceEntry : _knownOnlineDevices) {
            onDiscoveryFunc(deviceEntry.deviceDetails, DeviceEvent::ComeOnline);
        }
    }
}

void DeviceExplorer::setup() {
    _udp.listen(_portNo);
    _udp.onPacket([this](AsyncUDPPacket& packet) {
        this->_onPacket(packet);
    });
    _broadcastCountdown.start();
}

void DeviceExplorer::loop()
{
    if (_broadcastCountdown.hasExpired()) {
        _udp.broadcast((uint8_t*)&_thisDeviceMessage, sizeof(_thisDeviceMessage));
        _broadcastCountdown.restart();
    }

    if (_cleanUpCountdown.hasExpired()) {
        Ticks expiryTime(_elapsedTimer.elapsed() - _cleanUpCountdown.getCountdown());
        _removeOfflineDevices(expiryTime);
        _cleanUpCountdown.restart();
    }
}

void DeviceExplorer::_onPacket(AsyncUDPPacket& packet) 
{
    byte* data = packet.data();
    int length = packet.length();

    // Sending out broadcast packets, empirically they seem to 
    // not come back to the originator. However, this could be
    // implementation dependent, router behaviour dependence,
    // etc. So let's at least assert some consistency from this
    // point on by filtering out our own originating packets
    // (because if we introduce any downstream logic that 
    // assumes they'll be seen, that code could be contextually
    // fragile).

    if (packet.remoteIP() != packet.localIP()) // Not interested in ourselves.
    {
        // Log.warningln("DeviceExplorer: ignoring packets from ourselves.");

        if (length < sizeof(BinaryMessage)) {
             Log.errorln("DeviceExplorer: received invalid eRora packet (too small - size %d bytes).", length);
        } 
        
        else
        {
            BinaryMessage* message = reinterpret_cast<BinaryMessage*>(data);

            if (!message->headerIsValid()) {
                Log.errorln("DeviceExplorer: received invalid packet (not an eRora packet).");
            } 
            
            else {
                _handleMessage(packet.remoteIP(), message);
            }
        }
    }
}

void DeviceExplorer::_handleMessage(const IPAddress& originatingAddress, BinaryMessage* message) 
{
    if ((MessageType)(message->messageType) != MessageType::DeviceDiscovery) {
        Log.errorln("DeviceExplorer: received unexpected eRora message type (type %d).", (int)message->messageType);
    } 
    
    else
    {
        // DeviceDiscoveryMessage* deviceDiscoveryMessage = dynamic_cast<DeviceDiscoveryMessage*>(message);
        DeviceDiscoveryMessage* deviceDiscoveryMessage = (DeviceDiscoveryMessage*)message;

        if (deviceDiscoveryMessage) {
            _handleDiscoveryMessage(
                DeviceInformation(
                    originatingAddress, 
                    deviceDiscoveryMessage->getDeviceName(),
                    deviceDiscoveryMessage->getDeviceType()
                )
            );
        }
    }
}

void DeviceExplorer::_handleDiscoveryMessage(const DeviceInformation& deviceDetails)
{
    DeviceEvent event(_processDeviceMessage(deviceDetails));

    switch (event) {
        case DeviceEvent::ComeOnline:
            Log.verboseln("DeviceExplorer: informing observers of online device name \"%s\".", deviceDetails.name().c_str());
            _notifyOfOnlineDevice(deviceDetails);
            break;
        case DeviceEvent::Modified:
            Log.verboseln("DeviceExplorer: informing observers of device name change \"%s\".", deviceDetails.name().c_str());
            _notifyOfModifyDevice(deviceDetails);
            break;
        default:
            break;
    }
}

#if false

void DeviceExplorer::_handleInquiry(AsyncUDPPacket& packet) {
    // Log.verboseln("DeviceExplorer: received discovery inquiry from %s", packet.remoteIP().toString().c_str());
    // Log.verboseln("DeviceExplorer: sending discovery response with device name \"%s\".", _deviceName.c_str());
    _ResponseMessageBody messageBody(_deviceType, _deviceName);
    _ResponseMessage response(messageBody);
    AsyncUDPMessage message;
    message.write((uint8_t*)&response, sizeof(response));
    packet.send(message);
}

void DeviceExplorer::_handleResponse(AsyncUDPPacket& packet) {
    _ResponseMessage* responsePacket((_ResponseMessage*)(packet.data()));
    DeviceInformation deviceDetails(packet.remoteIP(), responsePacket->body.name(), responsePacket->body.getDeviceType());
    // Log.verboseln("DeviceExplorer: received discovery response with device name \"%s\".", deviceDetails.name().c_str());
    if (_addDevice(deviceDetails)) {
        Log.verboseln("DeviceExplorer: informing observers of online device name \"%s\".", deviceDetails.name().c_str());
        _notifyOfOnlineDevice(deviceDetails);
    }
}

#endif

DeviceExplorer::DeviceEvent DeviceExplorer::_processDeviceMessage(const DeviceInformation& deviceDetails)
{
    _ScopedLock lock(_mutex);

    DeviceEvent state(DeviceEvent::None);

    _WitnessedDevice onlineDevice(deviceDetails, _elapsedTimer.elapsed());
    IPAddress deviceAddress(deviceDetails.address());

    _WitnessedDevicesIter iter(_findDevice(deviceAddress));
    
    if (iter == _knownOnlineDevices.end()) {
        _knownOnlineDevices.push_back(onlineDevice);
        state = DeviceEvent::ComeOnline;
        Log.infoln("DeviceExplorer: found new eRora device named \"%s\" at %s.", deviceDetails.name().c_str(), deviceAddress.toString().c_str());
    }
    
    else {

        if ((deviceDetails.name() != iter->deviceDetails.name()) || (deviceDetails.category() != iter->deviceDetails.category())) {
            // Then the device changed name.
            state = DeviceEvent::Modified;
            iter->deviceDetails.setName(deviceDetails.name());
            iter->deviceDetails.setCategory(deviceDetails.category());
        }

        iter->lastSeenTime = _elapsedTimer.elapsed();
        // Log.verboseln("DeviceExplorer: refreshed eRora device named \"%s\" at %s.", deviceDetails.name().c_str(), deviceAddress.toString().c_str());
    }

    return state;
}

// DeviceExplorer::_WitnessedDevicesIter
void DeviceExplorer::_removeOfflineDevices(Ticks expiryTime) 
{
    _ScopedLock lock(_mutex);

    _WitnessedDevices removedDevices;

    _removeOfflineDevices(expiryTime, removedDevices);
    _notifyOfOfflineDevices(removedDevices); 
}

void DeviceExplorer::_removeOfflineDevices(Ticks expiryTime, _WitnessedDevices& removed) {
    removed.clear();
    _findStaleDevices(expiryTime, removed);
    _removeDevices(removed);
}

void DeviceExplorer::_findStaleDevices(Ticks expiryTime, _WitnessedDevices& staleDevices) {
    for (const _WitnessedDevice& device : _knownOnlineDevices) {
        if (device.lastSeenTime <= expiryTime) {
            staleDevices.push_back(device);
        }
    }
}

void DeviceExplorer::_removeDevices(_WitnessedDevices& devicesToRemove) {
    for (const _WitnessedDevice& device : devicesToRemove) {
        Log.infoln("DeviceExplorer: removed eRora device named \"%s\" at %s.", device.deviceDetails.name().c_str(), device.deviceDetails.address().toString().c_str());
        _knownOnlineDevices.remove(device);
    }
}

DeviceExplorer::_WitnessedDevicesIter DeviceExplorer::_findDevice(const IPAddress& deviceAddress) {
    return std::find_if(_knownOnlineDevices.begin(), _knownOnlineDevices.end(), 
        [deviceAddress](const _WitnessedDevice& item) {
            return item.deviceDetails.address() == deviceAddress;
        }
    );
}

void DeviceExplorer::_notifyOfOfflineDevices(const _WitnessedDevices& offlineDevices) {
    for (auto offlineDevice : offlineDevices) {
        _notifyOfOfflineDevice(offlineDevice.deviceDetails);
    }
}


void DeviceExplorer::_notifyOfOnlineDevice(const DeviceInformation& deviceDetails) {
    _notifyOfDeviceState(deviceDetails, DeviceEvent::ComeOnline);
}

void DeviceExplorer::_notifyOfModifyDevice(const DeviceInformation& deviceDetails) {
    _notifyOfDeviceState(deviceDetails, DeviceEvent::Modified);
}

void DeviceExplorer::_notifyOfOfflineDevice(const DeviceInformation& deviceDetails) {
    _notifyOfDeviceState(deviceDetails, DeviceEvent::GoneOffline);
}

void DeviceExplorer::_notifyOfDeviceState(const DeviceInformation& deviceDetails, DeviceEvent state) {
    for (auto observer : _observers) {
        observer(deviceDetails, state);
    }
}

// const DeviceExplorer::_InquiryMessage DeviceExplorer::_inquiryMessage;

