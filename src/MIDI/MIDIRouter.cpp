#include "MIDI/MIDIMonitor.h"

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

void MIDIMonitor::setup(const char* midiSessionName, uint16_t port)
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

void MIDIMonitor::loop() {
    _rtpSession.read();
    _midi.read();
}

void MIDIMonitor::setDeviceName(const std::string& deviceName) {
    _rtpSession.setName(deviceName.c_str());
}

std::string MIDIMonitor::getDeviceName() {
    return std::string(_rtpSession.getName());
}

void MIDIMonitor::addEventObserver(Observer* handler) {
    if (handler) {
        Log.verboseln("MIDIRouter: registering handler \"%s\".", handler->midiObserverName());
        _handlers.push_back(handler);
    }
}

void MIDIMonitor::removeEventObserver(Observer* handler) {
    if (handler) {
        Log.verboseln("MIDIRouter: deregistering handler \"%s\".", handler->midiObserverName());
        _handlers.remove(handler);
    }
}

void MIDIMonitor::_addSession(const _SessionID & sid, const char* name)
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

void MIDIMonitor::_removeSession(const _SessionID & sid)
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

void MIDIMonitor::_routeNoteOn(Channel channel, byte note, byte velocity) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, velocity](_Handler* handler) {
        if (handler) {
            handler->onMidiNoteOn(channel, note, velocity);
        }
    });
}

void MIDIMonitor::_routeNoteOff(Channel channel, byte note, byte velocity) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, velocity](_Handler* handler) {
        if (handler) {
            handler->onMidiNoteOff(channel, note, velocity);
        }
    });
}

void MIDIMonitor::_routeControlChange(Channel channel, byte type, byte value) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, type, value](_Handler* handler) {
        if (handler) {
            handler->onMidiControlChange(channel, type, value);
        }
    });
}

void MIDIMonitor::_routeProgramChange(Channel channel, byte patch) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, patch](_Handler* handler) {
        if (handler) {
            handler->onMidiProgramChange(channel, patch);
        }
    });
}

void MIDIMonitor::_routeAfterTouchChannel(Channel channel, byte pressure) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, pressure](_Handler* handler) {
        if (handler) {
            handler->onMidiAfterTouchChannel(channel, pressure);
        }
    });
}

void MIDIMonitor::_routeAfterTouchPoly(Channel channel, byte note, byte pressure) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, note, pressure](_Handler* handler) {
        if (handler) {
            handler->onMidiAfterTouchPoly(channel, note, pressure);
        }
    });
}

void MIDIMonitor::_routePitchBend(Channel channel, int bend) {
    std::for_each(_handlers.begin(), _handlers.end(), [channel, bend](_Handler* handler) {
        if (handler) {
            handler->onMidiPitchBend(channel, bend);
        }
    });
}

void MIDIMonitor::_routeSystemExclusive(byte* data, unsigned size) {
    std::for_each(_handlers.begin(), _handlers.end(), [data, size](_Handler* handler) {
        if (handler) {
            handler->onMidiSystemExclusive(data, size);
        }
    });
}

MIDIMonitor::_Session MIDIMonitor::_rtpSession("rtpMIDI-ESP32");
MIDIMonitor::_MIDI MIDIMonitor::_midi(_rtpSession);

MIDIMonitor::_Sessions MIDIMonitor::_sessions;
MIDIMonitor::_Handlers MIDIMonitor::_handlers;


