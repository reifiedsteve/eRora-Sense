#pragma once

#include "MIDI/MidiRouter.h"

class MidiDevice
{
protected:

    typedef MidiRouter _Router;
    typedef _Router::Handler _Handler;

public:

    // Not all interfaces have the notion of channel (e.g. MIDI-BLE)
    // but we include the notion here for those that do (others
    // can simply ignore it).

    typedef uint8_t byte;
    typedef _Handler Handler;

    virtual void setReceiveChannel(byte channel) = 0;
    virtual void setTransmitChannel(byte channel) = 0;

    virtual void addEventHandler(Handler* handler) = 0;
    virtual void removeEventHandler(Handler* handler) = 0;

    virtual void sendNoteOn(byte note, byte velocity) = 0;
    virtual void sendNoteOff(byte note, byte velocity) = 0;
    virtual void sendProgramChange(byte programNo) = 0;
    virtual void sendControlChange(byte type, byte value) = 0;
    virtual void sendAfterTouch(byte pressure) = 0;
    virtual void sendPolyPressure(byte note, byte pressure) = 0;
    virtual void sendPitchBend(int16_t bend) = 0;// MIDI spec says: -8192 to 8191
    virtual void sendSysEx(byte* data, unsigned size) = 0;

    virtual void loop() {}
};
