#pragma once

#include <MIDI.h>
#include <string>

class MIDIEventObserver 
{
public:

    typedef midi::Channel Channel;
    typedef uint8_t byte;

    virtual const char* midiObserverName() const { return "MIDI device"; }

    virtual void onMidiConnected(uint32_t sessionID, const std::string& sessionName) {}
    virtual void onMidiDisconnected(uint32_t sessionID, const std::string& sessionName) {}

    virtual void onMidiNoteOn(Channel channel, byte note, byte velocity) {}
    virtual void onMidiNoteOff(Channel channel, byte note, byte velocity) {}
    virtual void onMidiAfterTouchChannel(Channel channel, byte pressure) {}
    virtual void onMidiAfterTouchPoly(Channel channel, byte note, byte pressure) {}
    virtual void onMidiControlChange(Channel channel, byte type, byte value) {}
    virtual void onMidiProgramChange(Channel channel, byte patch) {}
    virtual void onMidiPitchBend(Channel channel, int bend) {}
    virtual void onMidiSystemExclusive(byte* data, unsigned size) {}

    virtual void setup() {}
    virtual void loop() {}
};

