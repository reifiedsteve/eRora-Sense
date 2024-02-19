#pragma once

#include <MIDI.hpp>
#include <string>

class MIDIHandler 
{
public:

    virtual const char* midiHandlerName() const = 0;

    virtual void midiHandleConnected(uint32_t sessionID, const std::string& sessionName) = 0;
    virtual void midiHandleDisconnected(uint32_t sessionID, const std::string& sessionName) = 0;

    virtual void midiHandleNoteOn(Channel channel, byte note, byte velocity) = 0;
    virtual void midiHandleNoteOff(Channel channel, byte note, byte velocity) = 0;
    virtual void midiHandleAfterTouchChannel(Channel channel, byte pressure) = 0;
    virtual void midiHandleAfterTouchPoly(Channel channel, byte note, byte pressure) = 0;
    virtual void midiHandleControlChange(Channel channel, byte type, byte value) = 0;
    virtual void midiHandleProgramChange(Channel channel, byte patch) = 0;
    virtual void midiHandlePitchBend(Channel channel, int bend) = 0;
    virtual void midiHandleSystemExclusive(byte* data, unsigned size) = 0;

    virtual void setup() = 0;
    virtual void loop() = 0;
};

