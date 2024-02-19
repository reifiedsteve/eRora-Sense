#pragma once

#include "MIDI/MIDIEventObserver.h"
#include "Diagnostics/Logging.h"

class MIDILogger : public MIDIEventObserver
{
public:

    const char* midiObserverName() const override {
        return "MIDI logging monitor";
    }

    void onMidiConnected(uint32_t sessionID, const std::string& sessionName) override {
        Log.verboseln("MIDI (RX): Connected - session %d, name \"%s\".", (int)sessionID, sessionName.c_str());
    }

    void onMidiDisconnected(uint32_t sessionID, const std::string& sessionName) override {
        Log.verboseln("MIDI (RX): Disonnected - session %d, name \"%s\".", (int)sessionID, sessionName.c_str());
    }

    void onMidiNoteOn(Channel channel, byte note, byte velocity) override {
        Log.verboseln("MIDI (RX): Note-On - channel %d, note %d (%s), velocity %d.", (int)channel, (int)note, _midiNoteNumberToNoteName(note).c_str(), (int)velocity);
    }

    void onMidiNoteOff(Channel channel, byte note, byte velocity) override {
        Log.verboseln("MIDI (RX): Note-Off - channel %d, note %d (%s), velocity %d.", (int)channel, (int)note, _midiNoteNumberToNoteName(note).c_str(), (int)velocity);
    }

    void onMidiAfterTouchChannel(Channel channel, byte pressure) override {
        Log.verboseln("MIDI (RX): After-Touch-Channel - channel %d, byte1 %d.", (int)channel, (int)pressure);
    }

    void onMidiAfterTouchPoly(Channel channel, byte note, byte pressure) override {
        Log.verboseln("MIDI (RX): After-Touch-Poly - channel %d, note %d (%s), pressure %d.", (int)channel, (int)note, _midiNoteNumberToNoteName(note).c_str(), (int)pressure);
    }

    void onMidiControlChange(Channel channel, byte type, byte value) override {
        // Meanings of specific CC#n type, see https://www.presetpatch.com/midi-cc-list.aspx 
        // e.g. CC#1 is modulation wheel input.
        //      CC#7 is volume.
        //      CC#8 is stereo panning.
        //      CC#10 also panning.
        Log.verboseln("MIDI (RX): Control-Change - channel %d, type %d, value %d.", (int)channel, (int)type, (int)value);
    }

    void onMidiProgramChange(Channel channel, byte patch) override {
        Log.verboseln("MIDI (RX): Program-Change - channel %d, patch %d.", (int)channel, (int)patch);
    };

    void onMidiPitchBend(Channel channel, int bend) override {
        Log.verboseln("MIDI (RX): Pitch-Bend -  channel %d, bend %d.", (int)channel, bend);
    }

    void onMidiSystemExclusive(byte* data, unsigned size) override {
        Log.verboseln("MIDI (RX): Sys-Ex - message of %d bytes.", (int)size);
    }

    void setup() override {

    }

    void loop() override {

    }

private:

    // See: https://inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies

    static std::string _midiNoteNumberToNoteName(int noteNo)
    {
        int bin(noteNo / 12);
        int octave(bin - 1);
        int note(noteNo - bin * 12);

        return _octaveAndNoteToNoteName(octave, note);
    }

    static std::string _octaveAndNoteToNoteName(int octave, int note)
    {
        static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

        // 0 is C-1
        // 12 is C0
        // 24 is C1
        // 36 is C2
        // etc.
        
        std::ostringstream ss;
        ss << noteNames[note] << octave;

        return ss.str();
    }
};

