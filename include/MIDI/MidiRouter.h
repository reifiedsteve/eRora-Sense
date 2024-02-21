#pragma once

#include "MIDI/MidiHandler.h"
#include <list>

class MidiRouter
{
public:

    // Note: Not all MIDI sessions have the notion of channels (e.g. MIDI-BLE)
    // and for those devices that do embody it, they work on just one channel.
    // Hence we allow handlers to assume the channel rather than include it
    // in every callback (which we cannot do for e.g. MIDI-BLE anyway).

    typedef MidiHandler Handler;

    void addEventHandler(Handler* handler);
    void removeEventHandler(Handler* handler);

    void onConnected(const std::string& sessionID);
    void onDisconnected(const std::string& sessionID);
    
    void onNoteOn(byte note, byte velocity);
    void onNoteOff(byte note, byte velocity);
    void onControlChange(byte type, byte value);
    void onProgramChange(byte patch);
    void onAfterTouchChannel(byte pressure);
    void onAfterTouchPoly(byte note, byte pressure);
    void onPitchBend(int bend); // MIDI spec says: -8192 to 8191
    void onSysEx(byte* data, unsigned size);

private:

    typedef Handler _Handler;
    typedef std::list<_Handler*> _Handlers;

    static _Handlers _handlers;
};

