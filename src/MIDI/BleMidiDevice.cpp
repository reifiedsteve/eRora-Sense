#include "MIDI/BleMidiDevice.h"

#include "Diagnostics/Logging.h"

#include <BLEMidi.h>

#include <sstream>

void BleMidiDevice::setReceiveChannel(byte channel) {
    _rxChannel = channel;
}

void BleMidiDevice::setTransmitChannel(byte channel) {
    _txChannel = channel;
}

void BleMidiDevice::addEventHandler(Handler* handler) {
    _router.addEventHandler(handler);
}

void BleMidiDevice::removeEventHandler(Handler* handler) {
    _router.removeEventHandler(handler);
}

void BleMidiDevice::sendNoteOn(byte note, byte velocity) {
    BLEMidiServer.noteOn(_txChannel, note, velocity);
}

void BleMidiDevice::sendNoteOff(byte note, byte velocity) {
    BLEMidiServer.noteOff(_txChannel, note, velocity);
}

void BleMidiDevice::sendProgramChange(byte programNo) {
    BLEMidiServer.programChange(_txChannel, programNo);
}

void BleMidiDevice::sendControlChange(byte type, byte value) {
    BLEMidiServer.controlChange(_txChannel, type, value);
}

void BleMidiDevice::sendAfterTouch(byte pressure) {
    BLEMidiServer.afterTouch(_txChannel, pressure);
}

void BleMidiDevice::sendPolyPressure(byte note, byte pressure) {
    BLEMidiServer.afterTouchPoly(_txChannel, note, pressure);
}

void BleMidiDevice::sendPitchBend(int16_t bend) { // MIDI spec says: -8192 to 8191
    BLEMidiServer.pitchBend(_txChannel, (uint16_t)bend);
}

void BleMidiDevice::sendSysEx(byte* data, unsigned size) {
    // Don't think unstructure data can go over BLE.
}

void BleMidiDevice::setup(const char* midiSessionName)
{
    // _bleSession.setName(midiSessionName);
    BLEMidiServer.begin(std::string(midiSessionName));

    // _rtpSession.begin();
    // _midi.begin(); // can add optional channel number here - defaults to 1.

    // _bleSession.setHandleConnected(_addSession);
    // _bleSession.setHandleDisconnected(_removeSession);

    BLEMidiServer.setOnConnectCallback(_onConnect);
    BLEMidiServer.setOnDisconnectCallback(_onDisconnect);
    
    BLEMidiServer.setNoteOnCallback(_onNoteOn);
    BLEMidiServer.setNoteOffCallback(_onNoteOff);
    BLEMidiServer.setControlChangeCallback(_onControlChange);
    BLEMidiServer.setProgramChangeCallback(_onProgramChange);
    BLEMidiServer.setAfterTouchCallback(_onAfterTouchChannel);
    BLEMidiServer.setAfterTouchPolyCallback(_onAfterTouchPoly);
    BLEMidiServer.setPitchBendCallback(_onPitchBend);
}

void BleMidiDevice::loop() 
{
    bool connected = BLEMidiServer.isConnected();

    if (connected && !_isConnected) {
        Log.verboseln("Bluetooth client connected.");
    } 
    
    else if (_isConnected && !connected) {
        Log.verboseln("Bluetooth client disconnected.");
    } 
    
    else {

    }

    _isConnected = connected;
}

void BleMidiDevice::_onConnect() {
    Log.verboseln("BleMidiDevice: Bluetooth client connected.");
}

void BleMidiDevice::_onDisconnect() {
    Log.verboseln("BleMidiDevice: Bluetooth client disconnected.");
}

void BleMidiDevice::_onNoteOn(byte channel, byte note, byte velocity, uint16_t timestamp) {
    if (_isChannelOfInterest(channel)) {
        _router.onNoteOn(note, velocity);
    }
}

void BleMidiDevice::_onNoteOff(byte channel, byte note, byte velocity, uint16_t timestamp) {
    if (_isChannelOfInterest(channel)) {
        _router.onNoteOff(note, velocity);
    }
}

void BleMidiDevice::_onControlChange(byte channel, byte type, byte value, uint16_t timestamp) {
    if (_isChannelOfInterest(channel)) {
        _router.onControlChange(type, value);
    }
}

void BleMidiDevice::_onProgramChange(byte channel, byte patch, uint16_t timestamp) {
    if (_isChannelOfInterest(channel)) {
        _router.onProgramChange(patch);
    }
}

void BleMidiDevice::_onAfterTouchChannel(byte channel, byte pressure, uint16_t timestamp) {
    if (_isChannelOfInterest(channel)) {
        _router.onAfterTouchChannel(pressure);
    }
}

void BleMidiDevice::_onAfterTouchPoly(byte channel, byte note, byte pressure, uint16_t timestamp) {
    if (_isChannelOfInterest(channel)) {
        _router.onAfterTouchPoly(note, pressure);
    }
}

void BleMidiDevice::_onPitchBend(byte channel, uint8_t lsb, uint8_t msb, uint16_t timestamp) {
    if (_isChannelOfInterest(channel)) {
        int bend = (msb << 7) | lsb;
        if (msb & 0b10000000) {
            bend = -((msb & 0b01111111) << 7) | lsb;
        }
        _router.onPitchBend(bend);
    }
}

bool BleMidiDevice::_isChannelOfInterest(byte channel) {
    return (_rxChannel == 0) || (channel == _rxChannel);
}

bool BleMidiDevice::_isConnected(false);

BleMidiDevice::byte BleMidiDevice::_rxChannel(1);
BleMidiDevice::byte BleMidiDevice::_txChannel(1);

// BleMidiDevice::_Session BleMidiDevice::_bleSession("bleMIDI-ESP32");
// BleMidiDevice::_MidiInterface BleMidiDevice::_midi(_bleSession);

// BleMidiDevice::_Connections BleMidiDevice::_connections;
BleMidiDevice::_Router BleMidiDevice::_router;
