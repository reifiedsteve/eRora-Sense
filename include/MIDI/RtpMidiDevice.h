#pragma once

#include "MIDI/MidiDevice.h"

#include <AppleMIDI.h>  // a.k.a  rtpMIDI
#include <WiFi.h>
#include <WiFiUDP.h>

#include <functional>
#include <map>
#include <list>

class RtpMidiDevice: public MidiDevice
{
private:

private:

    typedef APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP> _Session;
    typedef const APPLEMIDI_NAMESPACE::ssrc_t _ConnectionID;

    typedef MIDI_NAMESPACE::MidiInterface<_Session, APPLEMIDI_NAMESPACE::AppleMIDISettings> _MidiInterface;

public:

    RtpMidiDevice() 
    {}

    void setReceiveChannel(byte channel) override;
    void setTransmitChannel(byte channel) override;

    void addEventHandler(Handler* handler) override;
    void removeEventHandler(Handler* handler) override;

    void sendNoteOn(byte note, byte velocity) override;
    void sendNoteOff(byte note, byte velocity) override;
    void sendProgramChange(byte programNo) override;
    void sendControlChange(byte type, byte value) override;
    void sendAfterTouch(byte pressure) override;
    void sendPolyPressure(byte note, byte pressure) override;
    void sendPitchBend(int16_t bend) override; // MIDI spec says: -8192 to 8191
    void sendSysEx(byte* data, unsigned size) override;

    void setup(const char* midiSessionName = "rtpMIDI-ESP32", uint16_t port = DEFAULT_CONTROL_PORT);
    void loop() override;

private:

    static void _addSession(const _ConnectionID & sid, const char* name);
    static void _removeSession(const _ConnectionID & sid);

    static void _onNoteOn(byte channel, byte note, byte velocity);
    static void _onNoteOff(byte channel, byte note, byte velocity);
    static void _onControlChange(byte channel, byte type, byte value);
    static void _onProgramChange(byte channel, byte patch);
    static void _onAfterTouchChannel(byte channel, byte pressure);
    static void _onAfterTouchPoly(byte channel, byte note, byte pressure);
    static void _onPitchBend(byte channel, int bend);
    static void _onSystemExclusive(byte* data, unsigned size);

    static bool _isChannelOfInterest(byte channel);

    typedef std::map<_ConnectionID, std::string> _Connections;
    typedef _Connections::const_iterator _ConnectionsConstIter;
    typedef _Connections::iterator _ConnectionsIter;
    
    static byte _rxChannel, _txChannel;

    static _Session _rtpSession;
    static _MidiInterface _midi;

    static _Connections _connections; // Who's connected (remote rtp session name(s)).    

    static MidiRouter _router;
};

