#pragma once

#if 0

#include <AppleMIDI.h>

#include <WiFiUDP.h>

#include <sstream>

#include "Diagnostics/Logging.h"

class MIDIConnection
{
public:


    void setup()
    {
        AppleMIDI.setName("eRora-React-010203");
    
        // Start the MIDI service
        AppleMIDI.begin();
        MIDI.begin();
    
        const char* hostName = AppleMIDI.getName();
        int port = AppleMIDI.getPort();

        Serial.println("Host: ");
        Serial.println(String(hostName));

        Serial.print("Address: ");
        IPAddress addr = WiFi.localIP();
        Serial.println(addr);

        Serial.print("Port: ");
        Serial.println(port);

        // See: https://inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies

        // Stay informed on connection status
        AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
            // isConnected++;
            Log.verboseln("MIDI (Connect): \"%s\".", name);
        });

        AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
            // isConnected--;
            Log.verboseln("MIDI (Disconnect).");
        });

        MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
            Log.verboseln("MIDI (RX): Note-On - channel %d, note %d (%s), velocity %d.", (int)channel, (int)note, midiNoteToNoteName(note).c_str(), (int)velocity);
        });

        MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
            Log.verboseln("MIDI (RX): Note-Off - channel %d, note %d (%s), velocity %d.", (int)channel, (int)note, midiNoteToNoteName(note).c_str(), (int)velocity);
        });

        MIDI.setHandleAfterTouchPoly([](byte channel, byte note, byte velocity) {
            Log.verboseln("MIDI (RX): After-touch - channel %d, note %d (%s), velocity %d.", (int)channel, (int)note, midiNoteToNoteName(note).c_str(), (int)velocity);
        });

        MIDI.setHandleControlChange([](Channel channel, byte type, byte value) {
            // Meanings of specific CC#n type, see https://www.presetpatch.com/midi-cc-list.aspx 
            // e.g. CC#1 is modulation wheel input.
            //      CC#7 is volume.
            //      CC#8 is stereo panning.
            //      CC#10 also panning.
            Log.verboseln("MIDI (RX): Control-Change - channel %d, type %d, value %d.", (int)channel, (int)type, (int)value);
        });

        MIDI.setHandleProgramChange([](Channel channel, byte patch) {
            Log.verboseln("MIDI (RX): Program-Change - channel %d, patch %d.", (int)channel, (int)patch);
        });

        MIDI.setHandleAfterTouchChannel([](Channel channel, byte pressure) {
            Log.verboseln("MIDI (RX): After-Touch-Channel - channel %d, byte1 %d.", (int)channel, (int)pressure);
        });

        MIDI.setHandleAfterTouchPoly([](Channel channel, byte note, byte pressure) {
            Log.verboseln("MIDI (RX): After-Touch-Poly - channel %d, note %d (%s), pressure %d.", (int)channel, (int)note, midiNoteToNoteName(note).c_str(), (int)pressure);
        });

        MIDI.setHandlePitchBend([](Channel channel, int bend) {
            Log.verboseln("MIDI (RX): Pitch-Bend -  channel %d, bend %d.", (int)channel, bend);
        });

        MIDI.setHandleSystemExclusive([](byte* data, unsigned size) {
            Log.verboseln("MIDI (RX): Sys-Ex - message of %d bytes.", (int)size);
        });
    }

private:

    std::string _toNoteName(int noteNo)
    {
        static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

        // 0 is C-1
        // 12 is C0
        // 24 is C1

        int bin(noteNo / 12);
        int octave(bin - 1);
        int note(noteNo - bin * 12);

        std::ostringstream ss;
        ss << noteNames[note] << octave;

        return ss.str();
    }

};

#endif

void wifi_setup();
void midi_setup();
void midi_loop();
