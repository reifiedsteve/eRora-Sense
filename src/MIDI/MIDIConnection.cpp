// #include <Ethernet.h>
#include <AppleMIDI.h>

#include <WiFi.h>
#include <WiFiUDP.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE(); 

// AppleMIDI<WiFiUDP> appleMIDI;

void midi_setup()
{
  WiFi.begin("133381B", "spongebob2000");

  // Connect to the Wi-Fi network and start the captive portal
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("connected.");


  // ... (add your MIDI initialization code here)

  // Start the MIDI service
  AppleMIDI.begin();
  MIDI.begin();

  // Stay informed on connection status
  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
      // isConnected++;
      AM_DBG(F("Connected to session"), ssrc, name);
  });

  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
      // isConnected--;
      AM_DBG(F("Disconnected"), ssrc);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
      AM_DBG(F("NoteOn"), note);
  });

  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
      AM_DBG(F("NoteOff"), note);
  });
}

void midi_loop()
{
  
 
 }