#pragma once

#include "MIDI/MidiDevice.h"

class BleMidiDevice : public MidiDevice
{
private:

    // typedef BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_ESP32> _Session;
    // typedef MIDI_NAMESPACE::MidiInterface<BLEMIDI_NAMESPACE::BLEMIDI_Transport<BLEMIDI_NAMESPACE::BLEMIDI_ESP32>, BLEMIDI_NAMESPACE::MySettings> _MidiInterface;

public:

    // Not all interfaces have the notion of channel (e.g. MIDI-BLE)
    // but we include the notion here for those that do (others
    // can simply ignore it).

    typedef uint8_t byte;
    typedef _Handler Handler;

    void setReceiveChannel(byte channel) override;
    void setTransmitChannel(byte channel) override;

    void addEventHandler(Handler* handler) override;
    void removeEventHandler(Handler* handler) override;

    void sendNoteOn(byte note, byte velocity) override;
    void sendNoteOff(byte note, byte velocity) override;
    void sendProgramChange(byte programNo) override;
    void sendControlChange(byte type, byte value) override;
    void sendAfterTouch(byte pressure) override;
    void sendPolyPressure(byte note, byte pressure) override;
    void sendPitchBend(int16_t bend) override; // MIDI spec says: -8192 to 8191
    void sendSysEx(byte* data, unsigned size) override;

    void setup(const char* midiSessionName = "bleMIDI-ESP32");
    void loop() override;

private:

    static void _onConnect();
    static void _onDisconnect();
    
    static void _onNoteOn(byte channel, byte note, byte velocity, uint16_t timestamp);
    static void _onNoteOff(byte channel, byte note, byte velocity, uint16_t timestamp);
    static void _onControlChange(byte channel, byte type, byte value, uint16_t timestamp);
    static void _onProgramChange(byte channel, byte patch, uint16_t timestamp);
    static void _onAfterTouchChannel(byte channel, byte pressure, uint16_t timestamp);
    static void _onAfterTouchPoly(byte channel, byte note, byte pressure, uint16_t timestamp);
    static void _onPitchBend(byte channel, uint8_t lsb, uint8_t msb, uint16_t timestamp);

    static bool _isChannelOfInterest(byte channel);

    static bool _isConnected;

    static byte _rxChannel, _txChannel;

    //static _Session _bleSession;
    //static _MidiInterface _midi;
    
    static MidiRouter _router;
};

