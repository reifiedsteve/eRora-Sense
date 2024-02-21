#pragma once

#include <MIDI.h>
#include <string>

class MidiHandler 
{
public:

    typedef uint8_t byte;

    virtual const char* midiObserverName() const { return "MIDI device"; }

    virtual void onConnected(const std::string& sessionDescriptor) {}
    virtual void onDisconnected(const std::string& sessionDescriptor) {}

    virtual void onNoteOn(byte note, byte velocity) {}
    virtual void onNoteOff(byte note, byte velocity) {}
    virtual void onAfterTouchChannel(byte pressure) {}
    virtual void onAfterTouchPoly(byte note, byte pressure) {}
    virtual void onControlChange(byte type, byte value) {}
    virtual void onProgramChange(byte patch) {}
    virtual void onPitchBend(int bend) {}
    virtual void onSysEx(byte* data, unsigned size) {}

    virtual void setup() {}
    virtual void loop() {}
};

