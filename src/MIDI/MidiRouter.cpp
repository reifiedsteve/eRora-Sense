#include "MIDI/MidiRouter.h"
#include "Diagnostics/Logging.h"

void MidiRouter::addEventHandler(Handler* handler) {
    if (handler) {
        Log.verboseln("MIDIRouter: registering handler \"%s\".", handler->midiObserverName());
        _handlers.push_back(handler);
    }
}

void MidiRouter::removeEventHandler(Handler* handler) {
    if (handler) {
        Log.verboseln("MIDIRouter: deregistering handler \"%s\".", handler->midiObserverName());
        _handlers.remove(handler);
    }
}

void MidiRouter::onConnected(const std::string& sessionID) {
    std::for_each(_handlers.begin(), _handlers.end(), [sessionID](_Handler* handler) {
        if (handler) {
            handler->onConnected(sessionID);
        }
    });
}

void MidiRouter::onDisconnected(const std::string& sessionID) {
    std::for_each(_handlers.begin(), _handlers.end(), [sessionID](_Handler* handler) {
        if (handler) {
            handler->onDisconnected(sessionID);
        }
    });
}

void MidiRouter::onNoteOn(byte note, byte velocity) {
    std::for_each(_handlers.begin(), _handlers.end(), [note, velocity](_Handler* handler) {
        if (handler) {
            handler->onNoteOn(note, velocity);
        }
    });
}

void MidiRouter::onNoteOff(byte note, byte velocity) {
    std::for_each(_handlers.begin(), _handlers.end(), [note, velocity](_Handler* handler) {
        if (handler) {
            handler->onNoteOff(note, velocity);
        }
    });
}

void MidiRouter::onControlChange(byte type, byte value) {
    std::for_each(_handlers.begin(), _handlers.end(), [type, value](_Handler* handler) {
        if (handler) {
            handler->onControlChange(type, value);
        }
    });
}

void MidiRouter::onProgramChange(byte patch) {
    std::for_each(_handlers.begin(), _handlers.end(), [patch](_Handler* handler) {
        if (handler) {
            handler->onProgramChange(patch);
        }
    });
}

void MidiRouter::onAfterTouchChannel(byte pressure) {
    std::for_each(_handlers.begin(), _handlers.end(), [pressure](_Handler* handler) {
        if (handler) {
            handler->onAfterTouchChannel(pressure);
        }
    });
}

void MidiRouter::onAfterTouchPoly(byte note, byte pressure) {
    std::for_each(_handlers.begin(), _handlers.end(), [note, pressure](_Handler* handler) {
        if (handler) {
            handler->onAfterTouchPoly(note, pressure);
        }
    });
}

void MidiRouter::onPitchBend(int bend) {
    std::for_each(_handlers.begin(), _handlers.end(), [bend](_Handler* handler) {
        if (handler) {
            handler->onPitchBend(bend);
        }
    });
}

void MidiRouter::onSysEx(byte* data, unsigned size) {
    std::for_each(_handlers.begin(), _handlers.end(), [data, size](_Handler* handler) {
        if (handler) {
            handler->onSysEx(data, size);
        }
    });
}

MidiRouter::_Handlers MidiRouter::_handlers;

