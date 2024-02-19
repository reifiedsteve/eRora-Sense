#include "MIDI/MIDIRouter.h"

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

void MIDIRouter::setup(const char* midiSessionName, uint16_t port)
{
    _rtpSession.setName(midiSessionName);
    _rtpSession.setPort(port);

    _rtpSession.begin();
    _midi.begin();

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

void MIDIRouter::loop() {
    _rtpSession.read();
    _midi.read();
}

void MIDIRouter::setDeviceName(const std::string& deviceName) {
    _rtpSession.setName(deviceName.c_str());
}

std::string MIDIRouter::getDeviceName() {
    return std::string(_rtpSession.getName());
}

void MIDIRouter::addHandler(Handler* handler) {
    if (handler) {
        Log.verboseln("MIDIRouter: registering handler \"%s\".", handler->midiHandlerName());
        _handlers.push_back(handler);
    }
}

void MIDIRouter::removeHandler(Handler* handler) {
    if (handler) {
        Log.verboseln("MIDIRouter: deregistering handler \"%s\".", handler->midiHandlerName());
        _handlers.remove(handler);
    }
}

void MIDIRouter::_addSession(const _SessionID & sid, const char* name)
{
    _SessionsIter iter(_sessions.find(sid));

    if (iter == _sessions.end())
    {
        _sessions[sid] = name;

        std::for_each(_handlers.begin(), _handlers.end(), [sid, name](_Handler* handler) {
            if (handler) {
                handler->midiHandleConnected(sid, name);
            }
        });
    }

    else {
        Log.errorln("Reconnection from session (id %d) \"%s\".", sid, name);
    }
}

void MIDIRouter::_removeSession(const _SessionID & sid)
{
    _SessionsConstIter iter(_sessions.find(sid));

    if (iter != _sessions.end())
    {
        std::string sessionName(iter->second);
        _sessions.erase(sid);

        std::for_each(_handlers.begin(), _handlers.end(), [sid, sessionName](_Handler* handler) {
            if (handler) {
                handler->midiHandleDisconnected(sid, sessionName);
            }
        });
    }

    else {
        Log.errorln("Unknown session id %d.", sid);
    }
}

void MIDIRouter::_routeNoteOn(Channel channel, byte note, byte velocity) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, velocity](_Handler* handler) {
        if (handler) {
            handler->midiHandleNoteOn(channel, note, velocity);
        }
    });
}

void MIDIRouter::_routeNoteOff(Channel channel, byte note, byte velocity) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, velocity](_Handler* handler) {
        if (handler) {
            handler->midiHandleNoteOff(channel, note, velocity);
        }
    });
}

void MIDIRouter::_routeControlChange(Channel channel, byte type, byte value) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, type, value](_Handler* handler) {
        if (handler) {
            handler->midiHandleControlChange(channel, type, value);
        }
    });
}

void MIDIRouter::_routeProgramChange(Channel channel, byte patch) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, patch](_Handler* handler) {
        if (handler) {
            handler->midiHandleProgramChange(channel, patch);
        }
    });
}

void MIDIRouter::_routeAfterTouchChannel(Channel channel, byte pressure) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, pressure](_Handler* handler) {
        if (handler) {
            handler->midiHandleAfterTouchChannel(channel, pressure);
        }
    });
}

void MIDIRouter::_routeAfterTouchPoly(Channel channel, byte note, byte pressure) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, pressure](_Handler* handler) {
        if (handler) {
            handler->midiHandleAfterTouchPoly(channel, note, pressure);
        }
    });
}

void MIDIRouter::_routePitchBend(Channel channel, int bend) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, bend](_Handler* handler) {
        if (handler) {
            handler->midiHandlePitchBend(channel, bend);
        }
    });
}

void MIDIRouter::_routeSystemExclusive(byte* data, unsigned size) {
    std::for_each(_handlers.begin(), _handlers.end(), [data, size](_Handler* handler) {
        if (handler) {
            handler->midiHandleSystemExclusive(data, size);
        }
    });
}

MIDIRouter::_Session MIDIRouter::_rtpSession("rtpMIDI-ESP32");
MIDIRouter::_MIDI MIDIRouter::_midi(_rtpSession);

MIDIRouter::_Sessions MIDIRouter::_sessions;
MIDIRouter::_Handlers MIDIRouter::_handlers;


