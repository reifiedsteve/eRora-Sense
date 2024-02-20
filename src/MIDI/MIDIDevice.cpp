#include "MIDI/MIDIDevice.h"

#include "Diagnostics/Logging.h"

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

void RtpMidiConnection::setup(const char* midiSessionName, uint16_t port)
{
    _rtpSession.setName(midiSessionName);
    _rtpSession.setPort(port);

    _rtpSession.begin();
    _midi.begin(); // can add optional channel number here - defaults to 1.

    _rtpSession.setHandleConnected(_addSession);
    _rtpSession.setHandleDisconnected(_removeSession);

    _midi.setHandleNoteOn(_routeNoteOn);
    _midi.setHandleNoteOff(_routeNoteOff);
    _midi.setHandleControlChange(_routeControlChange);
    _midi.setHandleProgramChange(_routeProgramChange);
    _midi.setHandleAfterTouchChannel(_routeAfterTouchChannel);
    _midi.setHandleAfterTouchPoly(_routeAfterTouchPoly);
    _midi.setHandlePitchBend(_routePitchBend);
    _midi.setHandleSystemExclusive(_routeSystemExclusive);
}

void RtpMidiConnection::loop() {
    _rtpSession.read();
    _midi.read();
}

void RtpMidiConnection::setDeviceChannel(Channel channel) {
    _channel = channel;
    _midi.setInputChannel(_channel); // can add optional channel number here - defaults to 1.
}

void RtpMidiConnection::setDeviceName(const std::string& deviceName) {
    _rtpSession.setName(deviceName.c_str());
}

std::string RtpMidiConnection::getDeviceName() {
    return std::string(_rtpSession.getName());
}

void RtpMidiConnection::addEventObserver(Observer* handler) {
    if (handler) {
        Log.verboseln("MIDIRouter: registering handler \"%s\".", handler->midiObserverName());
        _handlers.push_back(handler);
    }
}

void RtpMidiConnection::removeEventObserver(Observer* handler) {
    if (handler) {
        Log.verboseln("MIDIRouter: deregistering handler \"%s\".", handler->midiObserverName());
        _handlers.remove(handler);
    }
}

void RtpMidiConnection::_addSession(const _SessionID & sid, const char* name)
{
    _SessionsIter iter(_sessions.find(sid));

    if (iter == _sessions.end())
    {
        _sessions[sid] = name;

        std::for_each(_handlers.begin(), _handlers.end(), [sid, name](_Handler* handler) {
            if (handler) {
                handler->onMidiConnected(sid, name);
            }
        });
    }

    else {
        Log.errorln("Reconnection from session (id %d) \"%s\".", sid, name);
    }
}

void RtpMidiConnection::_removeSession(const _SessionID & sid)
{
    _SessionsConstIter iter(_sessions.find(sid));

    if (iter != _sessions.end())
    {
        std::string sessionName(iter->second);
        _sessions.erase(sid);

        std::for_each(_handlers.begin(), _handlers.end(), [sid, sessionName](_Handler* handler) {
            if (handler) {
                handler->onMidiDisconnected(sid, sessionName);
            }
        });
    }

    else {
        Log.errorln("Unknown session id %d.", sid);
    }
}

void RtpMidiConnection::_routeNoteOn(Channel channel, byte note, byte velocity) {
    if (_isChannelOfInterest(channel)) {
        _notifyOfNoteOn(channel, note, velocity);
    }
}

void RtpMidiConnection::_notifyOfNoteOn(Channel channel, byte note, byte velocity) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, velocity](_Handler* handler) {
        if (handler) {
            handler->onMidiNoteOn(channel, note, velocity);
        }
    });
}

void RtpMidiConnection::_routeNoteOff(Channel channel, byte note, byte velocity) {
    if (_isChannelOfInterest(channel)) {
        _notifyOfNoteOff(channel, note, velocity);
    }
}

void RtpMidiConnection::_notifyOfNoteOff(Channel channel, byte note, byte velocity) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, velocity](_Handler* handler) {
        if (handler) {
            handler->onMidiNoteOff(channel, note, velocity);
        }
    });
}

void RtpMidiConnection::_routeControlChange(Channel channel, byte type, byte value) {
    if (_isChannelOfInterest(channel)) {
        _notifyOfControlChange(channel, type, value);
    }
}

void RtpMidiConnection::_notifyOfControlChange(Channel channel, byte type, byte value) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, type, value](_Handler* handler) {
        if (handler) {
            handler->onMidiControlChange(channel, type, value);
        }
    });
}

void RtpMidiConnection::_routeProgramChange(Channel channel, byte patch) {
    if (_isChannelOfInterest(channel)) {
        _notifyOfProgramChange(channel, patch);
    }
}

void RtpMidiConnection::_notifyOfProgramChange(Channel channel, byte patch) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, patch](_Handler* handler) {
        if (handler) {
            handler->onMidiProgramChange(channel, patch);
        }
    });
}

void RtpMidiConnection::_routeAfterTouchChannel(Channel channel, byte pressure) {
    if (_isChannelOfInterest(channel)) {
        _notifyOfAfterTouchChannel(channel, pressure);
    }
}

void RtpMidiConnection::_notifyOfAfterTouchChannel(Channel channel, byte pressure) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, pressure](_Handler* handler) {
        if (handler) {
            handler->onMidiAfterTouchChannel(channel, pressure);
        }
    });
}

void RtpMidiConnection::_routeAfterTouchPoly(Channel channel, byte note, byte pressure) {
    if (_isChannelOfInterest(channel)) {
        _notifyOfAfterTouchPoly(channel, note, pressure);
    }
}

void RtpMidiConnection::_notifyOfAfterTouchPoly(Channel channel, byte note, byte pressure) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, pressure](_Handler* handler) {
        if (handler) {
            handler->onMidiAfterTouchPoly(channel, note, pressure);
        }
    });
}

void RtpMidiConnection::_routePitchBend(Channel channel, int bend) {
    if (_isChannelOfInterest(channel)) {
        _notifyOfPitchBend(channel, bend);
    }
}

void RtpMidiConnection::_notifyOfPitchBend(Channel channel, int bend) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, bend](_Handler* handler) {
        if (handler) {
            handler->onMidiPitchBend(channel, bend);
        }
    });
}

void RtpMidiConnection::_routeSystemExclusive(byte* data, unsigned size) {
    _notifyOfSystemExclusive(data, size);
}

void RtpMidiConnection::_notifyOfSystemExclusive(byte* data, unsigned size) {
    std::for_each(_handlers.begin(), _handlers.end(), [data, size](_Handler* handler) {
        if (handler) {
            handler->onMidiSystemExclusive(data, size);
        }
    });
}

bool RtpMidiConnection::_isChannelOfInterest(Channel channel) {
    return (_channel == 0) || (channel == _channel);
}

RtpMidiConnection::_Session RtpMidiConnection::_rtpSession("rtpMIDI-ESP32");
RtpMidiConnection::_MIDI RtpMidiConnection::_midi(_rtpSession);

RtpMidiConnection::Channel RtpMidiConnection::_channel(1);

RtpMidiConnection::_Sessions RtpMidiConnection::_sessions;
RtpMidiConnection::_Handlers RtpMidiConnection::_handlers;


