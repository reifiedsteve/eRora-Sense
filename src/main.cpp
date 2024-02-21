/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#if 0

#include "Diagnostics/ScannerI2C.h"

ScannerI2C scanner;

void setup() {
  scanner.setup();
}

void loop() {
  scanner.loop();
}

#else

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#include <sstream>
#include <iomanip>

#include "Diagnostics/Logging.h"
#include "_Build/Version.h"

#include "Environment/BME680Sensor.h"
#include "Environment/LD2410Sensor.h"

#include "Environment/EnvironmentMonitor.h"
#include "Environment/EnvironmentLogger.h"

#include "MIDI/RtpMidiDevice.h"
#include "MIDI/BleMidiDevice.h"
#include "MIDI/MIDILogger.h"

#include <WiFi.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

#define LOCAL_SEALEVELPRESSURE_HPA (1027)

// Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

BME680Sensor envSensor;
LD2410Sensor personSensor(27);

EnvironmentMonitor envMonitor;
EnvironmentLogger envLogger;


#if 0

#include <functional>

class Foo
{
public:

    using NoteOnCallback = void (*)(byte channel, byte note, byte velocity);

    inline void setHandleNoteOn(NoteOnCallback fptr) { mNoteOnCallback = fptr; }

private:

    NoteOnCallback mNoteOnCallback;

};

class SubFoo : public Foo
{
public:

    typedef std::function(void(byte channel, byte note, byte velocity)) NoteOnObserver;

    inline void observeNoteOn(NoteOnObserver observer) {
        setHandleNoteOn(observer);
    }

};

#endif

void wifi_setup()
{
    WiFi.begin("133381B", "spongebob2000");

    // Connect to the Wi-Fi network and start the captive portal
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("connected.");
}

MIDILogger midiMonitor;

RtpMidiDevice rtpMidi;
//BleMidiDevice bleMidi;

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    initLogging(LOG_LEVEL_VERBOSE);

    std::ostringstream os;
    os << "Built: " << BUILD_DATE << "@" << BUILD_TIME;
    Log.infoln(os.str().c_str());

    wifi_setup();

    rtpMidi.setup("eRora-MIDI-rtp");
    rtpMidi.addEventHandler(&midiMonitor);
    //bleMidi.setup("eRora-MIDI-ble");
    //bleMidi.addEventHandler(&midiMonitor);
/**
  if (!bme.begin()) {
    Log.errorln("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
*/

/*
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
*/

/***
 
  envSensor.setup();
  personSensor.setup();

  envMonitor.setup();
  // envLogger.setup();

  envMonitor.attachOccupancySensor(personSensor);
  envMonitor.attachTemperatureSensor(envSensor);
  envMonitor.attachHumiditySensor(envSensor);
  
  envMonitor.addOccupancyObserver(envLogger);
  envMonitor.addTemperatureObserver(envLogger);
  envMonitor.addHumidityObserver(envLogger);

  ***/
}

void loop() 
{
# if 0

  if (! bme.performReading()) {
    Log.errorln("Failed to perform reading :(");
    return;
  }

# endif

# if 0

  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

# endif

# if 0

  std::ostringstream os;

  os << "Temperature: " << std::setprecision(1) << std::fixed << bme.temperature << " *C";
  os << ", ";
  os << "Pressure: " << (int) (bme.pressure / 100.0) << " hPa";
  os << ", ";
  os << "Humidity " << std::setprecision(1) << std::fixed << bme.humidity << " %";
  os << ", ";
  // os << "Gas " << (int)(bme.gas_resistance / 1000.0) << " KOhms";
  os << "Gas: " << (int)(bme.gas_resistance / 1000.0) << " KOhms";
  os << ", ";
  os << "Alt: " << bme.readAltitude(LOCAL_SEALEVELPRESSURE_HPA) << " m";
  /*
  os << ", ";
  os << "Occupied: " << (env.occupied() ? "Yes" : "No");
  */
  // os << std::endl;

  // Log.infoln("%s", String(os.str().c_str()));
  Log.verboseln("%s", os.str().c_str());

  env.loop();

# endif


/*
envMonitor.attachTemperatureSensor(envSensor);
envMonitor.addTemperatureObserver(envLogger, true);
*/


/***
 
envSensor.loop();
personSensor.loop();

envMonitor.loop();

***/

midiMonitor.loop();

rtpMidi.loop();
//bleMidi.loop();

  // delay(2000);
}

#endif
