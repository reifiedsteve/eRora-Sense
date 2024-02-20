#pragma once

#include <functional>
#include <map>
#include <list>

#include <AppleMIDI.h>  // a.k.a  rtpMIDI
#include <WiFi.h>
#include <WiFiUDP.h>

#include "MIDI/MIDIEventObserver.h"

class RtpMidiConnection
{
private:

    typedef APPLEMIDI_NAMESPACE::AppleMIDISession<WiFiUDP> _Session;
    typedef const APPLEMIDI_NAMESPACE::ssrc_t _SessionID;

    typedef MIDI_NAMESPACE::MidiInterface<_Session, APPLEMIDI_NAMESPACE::AppleMIDISettings> _MIDI;

public:
    
    typedef midi::Channel Channel;
    typedef MIDIEventObserver Observer;

    static void setup(const char* midiSessionName = "rtpMIDI-ESP32", uint16_t port = DEFAULT_CONTROL_PORT);
    static void loop();

    static void setDeviceChannel(Channel channel);

    static void setDeviceName(const std::string& deviceName);
    static std::string getDeviceName();

    static void addEventObserver(Observer* handler);
    static void removeEventObserver(Observer* handler);

# if 0

    // TODO: add controller methods here. We may be implementing an instrucment.

    inline static void sendNoteOn(byte note, byte velocity = 64, byte channel = 1) __attribute__((always_inline)) {
        _midi.sendNoteOn(note, velocity, channel);
    }

    inline static void sendNoteOff(byte note, byte velocity = 64, byte channel = 1) __attribute__((always_inline)) {
        _midi.sendNoteOff(note, velocity, channel);
    }

# endif

    RtpMidiConnection() = delete;

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

    static void _notifyOfNoteOn(Channel channel, byte note, byte velocity);
    static void _notifyOfNoteOff(Channel channel, byte note, byte velocity);
    static void _notifyOfControlChange(Channel channel, byte type, byte value);
    static void _notifyOfProgramChange(Channel channel, byte patch);
    static void _notifyOfAfterTouchChannel(Channel channel, byte pressure);
    static void _notifyOfAfterTouchPoly(Channel channel, byte note, byte pressure);
    static void _notifyOfPitchBend(Channel channel, int bend);
    static void _notifyOfSystemExclusive(byte* data, unsigned size);

    static bool _isChannelOfInterest(Channel channel);

    typedef std::map<_SessionID, std::string> _Sessions;
    typedef _Sessions::const_iterator _SessionsConstIter;
    typedef _Sessions::iterator _SessionsIter;
    
    typedef Observer _Handler;
    typedef std::list<_Handler*> _Handlers;

    static _Session _rtpSession;
    static _MIDI _midi;

    static Channel _channel;

    static _Sessions _sessions; // Who's connected (remote rtp sessio name(s)).    
    static _Handlers _handlers;
};

class MidiSession
{
public:

    typedef uint8_t byte;
    typedef MIDIEventObserver Observer;

    void setDeviceChannel(Channel channel);

    void addEventObserver(Observer* handler);
    void removeEventObserver(Observer* handler);

    virtual void sendNoteOn(byte note, byte velocity) = 0;
    virtual void sendNoteOff(byte note, byte velocity) = 0;
    // etc...

    virtual void loop() {}

protected:

    MidiSession() 
      : _channel(1)
    {}

    Channel _channel;
};

#if 0

class RtpMidiSession: public MidiSession
{
private:

public:

    void setup(const char* midiSessionName = "rtpMIDI-ESP32") {
        RtpMidiConnection::setDeviceName(midiSessionName);
    }

    void addEventObserver(Observer* handler);
    void removeEventObserver(Observer* handler);

    virtual void sendNoteOn(Channel channel, byte note, byte velocity) = 0;
    virtual void sendNoteOff(Channel channel, byte note, byte velocity) = 0;

    void loop() override;
};

class BleMidiSession : public MidiSession
{

};

class MidiDevice
{
public:

    enum class Protocol {
        TransportRtpMidi,
        TransportBleMidi
    };


    void selectTransport(Protocol protocol)
    {
        if (protocol != _protocol)
        {
            switch (_protocol) {
                case Protocol::TransportRtpMidi:
                    // TODO: switch off WiFI.
                    _session = nullptr;
                    break;
                case Protocol::TransportBleMidi:
                    // TODO: switch off BLE.
                    _session = nullptr;
                default:
                    // TODO: switch off WiFI.
                    // TODO: switch off BLE.
                    _session = nullptr;

            }

            _protocol = protocol;

            switch (_protocol) {
                case Protocol::TransportRtpMidi:
                    // TODO: switch on WiFI.
                    _session = &_rtpMidi;
                    break;
                case Protocol::TransportBleMidi:
                    // TODO: switch on BLE.
                    _session = &_bleMidi;
                default:
                    break;
            }

            // TODO: Might need to store selection non-volatile and reboot?
        }
    }

private:

    Protocol _protocol;

    RtpMidiSession _rtpMidi;
    BleMidiSession _bleMidi;
    MidiSession* _session;
};

#endif
