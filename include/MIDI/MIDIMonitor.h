#pragma once

#include <functional>
#include <map>
#include <list>

#include <AppleMIDI.h>
#include <WiFi.h>
#include <WiFiUDP.h>

#include "MIDI/MIDIEventObserver.h"

class MIDIMonitor   // TODO: rename to MIDIRouterRTP
{
private:

    typedef APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP> _Session;
    typedef const APPLEMIDI_NAMESPACE::ssrc_t _SessionID;

    typedef MIDI_NAMESPACE::MidiInterface<_Session, APPLEMIDI_NAMESPACE::AppleMIDISettings> _MIDI;

public:
    
    typedef MIDIEventObserver Observer;

    static void setup(const char* midiSessionName = "rtpMIDI-ESP32", uint16_t port = DEFAULT_CONTROL_PORT);
    static void loop();

    static void setDeviceName(const std::string& deviceName);
    static std::string getDeviceName();

    static void addEventObserver(Observer* handler);
    static void removeEventObserver(Observer* handler);

    MIDIMonitor() = delete;

private:

    static void _addSession(const _SessionID & sid, const char* name);
    static void _removeSession(const _SessionID & sid);

    static void _routeNoteOn(Channel channel, byte note, byte velocity);
    static void _routeNoteOff(Channel channel, byte note, byte velocity);
    static void _routeControlChange(Channel channel, byte type, byte value);
    static void _routeProgramChange(Channel channel, byte patch);
    static void _routeAfterTouchChannel(Channel channel, byte pressure);
    static void _routeAfterTouchPoly(Channel channel, byte note, byte pressure);
    static void _routePitchBend(Channel channel, int bend);
    static void _routeSystemExclusive(byte* data, unsigned size);

    typedef std::map<_SessionID, std::string> _Sessions;
    typedef _Sessions::const_iterator _SessionsConstIter;
    typedef _Sessions::iterator _SessionsIter;
    
    typedef Observer _Handler;
    typedef std::list<_Handler*> _Handlers;

    static _Session _rtpSession;
    static _MIDI _midi;

    static _Sessions _sessions; // Who's connected (remote rtp sessio name(s)).    
    static _Handlers _handlers;
};
