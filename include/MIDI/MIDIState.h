#pragma once

#include "MIDI/MIDIEventObserver.h"
#include <array>

typedef unsigned short uint7_t;

class ChannelState 
{
public:

    explicit ChannelState(byte bend = 0, byte pressure = 0)
      : _bend(bend)
      , _pressure(pressure)
    {}

    inline byte bend() const { return _bend; }
    inline byte pressure() const { return _pressure; }

    inline void setBend(byte value) { _bend = value; }
    inline void setPressure(byte value) { _pressure = value; }

private:

    uint7_t _bend;
    uint7_t _pressure; // channel aftertouch.
};

class NoteState
{
public:

    explicit NoteState(bool on = false, uint7_t velocity = 0, uint7_t pressure = 0)
      : _on(on)
      , _velocity(velocity)
      , _pressure(pressure)
    {}

    inline uint7_t isOn() const { return _on; }
    inline uint7_t velocity() const { return _velocity; }
    inline uint7_t pressure() const { return _pressure; }

    inline void setPressure(byte value) { _pressure = value; }

private:

    bool _on;
    uint7_t _velocity;
    uint7_t _pressure; // MPE, poly aftertouch.
    // uint7_t _slide; // MPE
    // uint7_t _modulation; // MPE, driven by CC#11.
    // uint7_t _bend; // MPE, CC#14.
};

/// @brief Here, a device runs on a single midi channel.
class MidiDeviceState
{
public:

    void reset() {
        std::fill(std::begin(_noteStates), std::end(_noteStates), NoteState());
        std::fill(std::begin(_controlParameters), std::end(_controlParameters), 0);
        _channelState = ChannelState();
    }

    void noteOn(byte note, byte velocity) {
        _noteStates[note] = NoteState(true, velocity);
    }

    void noteOff(byte note, byte velocity) {
        _noteStates[note] = NoteState(false, velocity);
    }

    void afterTouch(byte pressure) {
        _channelState.setPressure(pressure);
    }

    void afterTouchPoly(byte note, byte pressure) {
        _noteStates[note].setPressure(pressure);
    }

    void controlChange(byte type, byte value) {
        _controlParameters[type] = value;
    }

    // void onMidiProgramChange(Channel channel, byte patch) override {}

    void pitchBend(int bend) {
       _channelState.setBend(bend); 
    }
    
    // void onMidiSystemExclusive(byte* data, unsigned size) override {}

    // void setup() override {}
    // void loop() override {}

private:

    std::array<NoteState, 128> _noteStates;
    ChannelState _channelState;
    std::array<uint7_t, 128> _controlParameters;
};

#if 0

class MidiDevice
{
public:

    void setConnection(MidiConnection& connection) {

    }

private:

    byte _channel;
    MidiDeviceState _state;
};

#endif
