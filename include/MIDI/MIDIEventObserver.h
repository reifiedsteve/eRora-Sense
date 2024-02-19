#pragma once

#include <MIDI.hpp>
#include <string>

class MIDIEventObserver 
{
public:

    virtual const char* midiObserverName() const = 0;

    virtual void onMidiConnected(uint32_t sessionID, const std::string& sessionName) = 0;
    virtual void onMidiDisconnected(uint32_t sessionID, const std::string& sessionName) = 0;

    virtual void onMidiNoteOn(Channel channel, byte note, byte velocity) = 0;
    virtual void onMidiNoteOff(Channel channel, byte note, byte velocity) = 0;
    virtual void onMidiAfterTouchChannel(Channel channel, byte pressure) = 0;
    virtual void onMidiAfterTouchPoly(Channel channel, byte note, byte pressure) = 0;
    virtual void onMidiControlChange(Channel channel, byte type, byte value) = 0;
    virtual void onMidiProgramChange(Channel channel, byte patch) = 0;
    virtual void onMidiPitchBend(Channel channel, int bend) = 0;
    virtual void onMidiSystemExclusive(byte* data, unsigned size) = 0;

    virtual void setup() = 0;
    virtual void loop() = 0;
};

