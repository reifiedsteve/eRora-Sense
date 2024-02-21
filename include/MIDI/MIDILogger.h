#pragma once

#include "MIDI/MidiHandler.h"
#include "Diagnostics/Logging.h"

#include <sstream>

class MIDILogger : public MidiHandler
{
public:

    const char* midiObserverName() const override {
        return "MIDI logging monitor";
    }

    void onConnected(const std::string& sessionDescriptor) override {
        Log.verboseln("MIDI (RX): Connected - session \"%s\".", sessionDescriptor.c_str());
    }

    void onDisconnected(const std::string& sessionDescriptor) override {
        Log.verboseln("MIDI (RX): Disonnected - session \"%s\".", sessionDescriptor.c_str());
    }

    void onNoteOn(byte note, byte velocity) override {
        Log.verboseln("MIDI (RX): Note-On - note %d (%s), velocity %d.", (int)note, _midiNoteNumberToNoteName(note).c_str(), (int)velocity);
    }

    void onNoteOff(byte note, byte velocity) override {
        Log.verboseln("MIDI (RX): Note-Off - note %d (%s), velocity %d.", (int)note, _midiNoteNumberToNoteName(note).c_str(), (int)velocity);
    }

    void onAfterTouchChannel(byte pressure) override {
        Log.verboseln("MIDI (RX): After-Touch-Channel - pressure %d.", (int)pressure);
    }

    void onAfterTouchPoly(byte note, byte pressure) override {
        Log.verboseln("MIDI (RX): After-Touch-Poly - note %d (%s), pressure %d.", (int)note, _midiNoteNumberToNoteName(note).c_str(), (int)pressure);
    }

    void onControlChange(byte type, byte value) override {
        // Meanings of specific CC#n type, see https://www.presetpatch.com/midi-cc-list.aspx 
        // e.g. CC#1 is modulation wheel input.
        //      CC#7 is volume.
        //      CC#8 is stereo panning.
        //      CC#10 also panning.
        Log.verboseln("MIDI (RX): Control-Change - type %d, value %d.", (int)type, (int)value);
    }

    void onProgramChange(byte patch) override {
        Log.verboseln("MIDI (RX): Program-Change - patch %d.", (int)patch);
    };

    void onPitchBend(int bend) override {
        Log.verboseln("MIDI (RX): Pitch-Bend -  bend %d.", bend);
    }

    void onSysEx(byte* data, unsigned size) override {
        Log.verboseln("MIDI (RX): Sys-Ex - message of %d bytes.", (int)size);
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

