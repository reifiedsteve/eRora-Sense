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

#include <Arduino.h>

#include "Diagnostics/Logging.h"
#include "_Build/Version.h"

#include "Sensors/BME680Sensor.h"
#include "Sensors/LD2410Sensor.h"
#include "Sensors/PMS7003ParticleSensor.h"

#include "Sensors/EnvironmentMonitor.h"
#include "Sensors/EnvironmentLogger.h"

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
PMS7003ParticleSensor pmSensor(16, 17, TimeSpan::fromSeconds(5)); // TODO: make say 10 minutes (to avoid sensor wearing out). 

EnvironmentMonitor envMonitor;
EnvironmentLogger envLogger;


void wifi_setup()
{
    WiFi.begin("133381B", "spongebob2000");

    // Connect to the Wi-Fi network and start the captive portal
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("WiFi connected.");
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    initLogging(LOG_LEVEL_VERBOSE);

    Log.infoln("eRora Sense");

    std::ostringstream os;
    os << "Built: " << BUILD_DATE << "@" << BUILD_TIME;
    Log.infoln(os.str().c_str());

    wifi_setup();

    envSensor.setup();
    personSensor.setup();
    pmSensor.setup();

    envMonitor.setup();
    // envLogger.setup();

    bool reportInitial(true);

    envMonitor.addTemperatureObserver(envLogger, reportInitial);
    envMonitor.addHumidityObserver(envLogger, reportInitial);
    envMonitor.addGasLevelObserver(envLogger, reportInitial);
    envMonitor.addParticleObserver(envLogger, reportInitial);
    envMonitor.addOccupancyObserver(envLogger, reportInitial);

    envMonitor.attachTemperatureSensor(envSensor);
    envMonitor.attachHumiditySensor(envSensor);
    envMonitor.attachGasLevelSensor(envSensor);
    envMonitor.attachParticleSensor(pmSensor);
    envMonitor.attachOccupancySensor(personSensor);
}

CountdownTimer timer(TimeSpan::fromSeconds(5).millis(), CountdownTimer::State::Running);
static int loops(0);

void loop()
{
    #if 0

    if (timer.hasExpired()) {
      Log.verboseln("Looped %d times...", loops);
      timer.restart();
    }

    ++loops;

    #endif

    envSensor.loop();
    personSensor.loop();
    pmSensor.loop();

    envMonitor.loop();
}

#endif
