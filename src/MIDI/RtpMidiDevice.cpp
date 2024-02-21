#include "MIDI/RtpMidiDevice.h"

#include "Diagnostics/Logging.h"
#include <sstream>

// APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE(); // expands to following line...
//
// APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "AppleMIDI-ESP32", DEFAULT_CONTROL_PORT); // which expands to the following two lines.
//
// APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP> AppleMIDI("AppleMIDI-ESP32", DEFAULT_CONTROL_PORT); 
// MIDI_NAMESPACE::MidiInterface<APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP>, APPLEMIDI_NAMESPACE::AppleMIDISettings> MIDI((APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP> &)AppleMIDI);
//
// ...and lets not start instances with a capital...   So appleMIDI (not AppleMIDI) and midiSession (not MIDI).
// APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP> appleMIDI("AppleMIDI-ESP32", DEFAULT_CONTROL_PORT);
// MIDI_NAMESPACE::MidiInterface<APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP>, APPLEMIDI_NAMESPACE::AppleMIDISettings> midiSession((APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP> &)appleMIDI);

void RtpMidiDevice::setReceiveChannel(byte channel) {
    _rxChannel = channel;
    _midi.setInputChannel(_rxChannel); // can add optional channel number here - defaults to 1.
}

void RtpMidiDevice::setTransmitChannel(byte channel) {
    _txChannel = channel;
}

void RtpMidiDevice::addEventHandler(Handler* handler) {
    _router.addEventHandler(handler);
}

void RtpMidiDevice::removeEventHandler(Handler* handler) {
    _router.removeEventHandler(handler);
}

void RtpMidiDevice::sendNoteOn(byte note, byte velocity) {
    _midi.sendNoteOn(note, velocity, _txChannel);
}

void RtpMidiDevice::sendNoteOff(byte note, byte velocity) {
    _midi.sendNoteOff(note, velocity, _txChannel);
}

void RtpMidiDevice::sendProgramChange(byte programNo) {
    _midi.sendProgramChange(programNo, _txChannel);
}

void RtpMidiDevice::sendControlChange(byte type, byte value) {
    _midi.sendControlChange(type, value, _txChannel);
}

void RtpMidiDevice::sendAfterTouch(byte pressure) {
    _midi.sendAfterTouch(pressure, _txChannel);
}

void RtpMidiDevice::sendPolyPressure(byte note, byte pressure) {
    _midi.sendPolyPressure(note, pressure, _txChannel);
}

void RtpMidiDevice::sendPitchBend(int16_t bend) {
    _midi.sendPitchBend(bend, _txChannel);
}

void RtpMidiDevice::sendSysEx(byte* data, unsigned size) {
    _midi.sendSysEx(size, data, false);
}

void RtpMidiDevice::setup(const char* midiSessionName, uint16_t port)
{
    _rtpSession.setName(midiSessionName);
    _rtpSession.setPort(port);

    _rtpSession.begin();
    _midi.begin(); // can add optional channel number here - defaults to 1.

    _rtpSession.setHandleConnected(_addSession);
    _rtpSession.setHandleDisconnected(_removeSession);

    _midi.setHandleNoteOn(_onNoteOn);
    _midi.setHandleNoteOff(_onNoteOff);
    _midi.setHandleControlChange(_onControlChange);
    _midi.setHandleProgramChange(_onProgramChange);
    _midi.setHandleAfterTouchChannel(_onAfterTouchChannel);
    _midi.setHandleAfterTouchPoly(_onAfterTouchPoly);
    _midi.setHandlePitchBend(_onPitchBend);
    _midi.setHandleSystemExclusive(_onSystemExclusive);
}

void RtpMidiDevice::loop() {
    _rtpSession.read();
    _midi.read();
}

void RtpMidiDevice::_addSession(const _ConnectionID & sid, const char* name)
{
    _ConnectionsIter iter(_connections.find(sid));

    if (iter == _connections.end())
    {
        std::stringstream ss;
        ss << name << " [" << sid << "]";

        _router.onConnected(ss.str());        
        _connections[sid] = name;
    }

    else {
        Log.warningln("Reconnection from session (id %d) \"%s\".", sid, name);
    }
}

void RtpMidiDevice::_removeSession(const _ConnectionID & sid)
{
    _ConnectionsConstIter iter(_connections.find(sid));

    if (iter != _connections.end())
    {
        std::string sessionName(iter->second);
        _connections.erase(sid);

        std::stringstream ss;
        ss << sessionName << " [" << sid << "]";

        _router.onDisconnected(ss.str());
    }

    else {
        Log.warningln("Unknown session id %d.", sid);
    }
}

void RtpMidiDevice::_onNoteOn(byte channel, byte note, byte velocity) {
    if (_isChannelOfInterest(channel)) {
        _router.onNoteOn(note, velocity);
    }
}

void RtpMidiDevice::_onNoteOff(byte channel, byte note, byte velocity) {
    if (_isChannelOfInterest(channel)) {
        _router.onNoteOff(note, velocity);
    }
}

void RtpMidiDevice::_onControlChange(byte channel, byte type, byte value) {
    if (_isChannelOfInterest(channel)) {
        _router.onControlChange(type, value);
    }
}

void RtpMidiDevice::_onProgramChange(byte channel, byte patch) {
    if (_isChannelOfInterest(channel)) {
        _router.onProgramChange(patch);
    }
}

void RtpMidiDevice::_onAfterTouchChannel(byte channel, byte pressure) {
    if (_isChannelOfInterest(channel)) {
        _router.onAfterTouchChannel(pressure);
    }
}

void RtpMidiDevice::_onAfterTouchPoly(byte channel, byte note, byte pressure) {
    if (_isChannelOfInterest(channel)) {
        _router.onAfterTouchPoly(note, pressure);
    }
}

void RtpMidiDevice::_onPitchBend(byte channel, int bend) {
    if (_isChannelOfInterest(channel)) {
        _router.onPitchBend(bend);
    }
}

void RtpMidiDevice::_onSystemExclusive(byte* data, unsigned size) {
    _router.onSysEx(data, size);
}

bool RtpMidiDevice::_isChannelOfInterest(byte channel) {
    return (_rxChannel == 0) || (channel == _rxChannel);
}

RtpMidiDevice::byte RtpMidiDevice::_rxChannel(1);
RtpMidiDevice::byte RtpMidiDevice::_txChannel(1);

RtpMidiDevice::_Session RtpMidiDevice::_rtpSession("rtpMIDI-ESP32");
RtpMidiDevice::_MidiInterface RtpMidiDevice::_midi(_rtpSession);

RtpMidiDevice::_Connections RtpMidiDevice::_connections;
RtpMidiDevice::_Router RtpMidiDevice::_router;
