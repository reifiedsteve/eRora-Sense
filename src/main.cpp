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

#include <WiFi.h>
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
#include "Sensors/SGP30Sensor.h"
// #include "Sensors/LD2410Sensor.h"
#include "Sensors/PMS7003ParticleSensor.h"

#include "Sensors/EnvironmentMonitor.h"
#include "Sensors/EnvironmentLogger.h"

#include "Diagnostics/ScannerI2C.h"
#include "Chronos/CountdownTimer.h"

#include "Displays/SD1602/CharacterMatrix1602.h"

/*
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
*/

#define SEALEVELPRESSURE_HPA (1013.25)

#define LOCAL_SEALEVELPRESSURE_HPA (1027)

// Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

BME680Sensor bmeSensor;
SGP30Sensor sgpSensor;

// LD2410Sensor personSensor(27);
PMS7003ParticleSensor pmSensor(16, 17, TimeSpan::fromSeconds(5)); // TODO: make say 10 minutes (to avoid sensor wearing out).

EnvironmentMonitor envMonitor;
EnvironmentLogger envLogger;

//#if 0
CharacterMatrix1602 lcd;
//#else
//LiquidCrystal_I2C _lcd(0x27, 16, 2);
//#endif

/// ScannerI2C scanner;


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

byte charFanBlades0[] = {
    0b00000,
    0b11011,
    0b10110,
    0b10100,
    0b11111,
    0b00101,
    0b01101,
    0b11011
};

byte charFanBlades1[] = {
    0b11100,
    0b01100,
    0b00101,
    0b11111,
    0b11111,
    0b00110,
    0b01101,
    0b00111
};

byte charFanBlades2[] = {
    0b01110,
    0b11111,
    0b01110,
    0b00100,
    0b01110,
    0b11111,
    0b01110,
    0b00000
};

byte charFanBlades[] = {
    0b00000,
    0b01000,
    0b10101,
    0b00010,
    0b01000,
    0b10101,
    0b00010,
    0b00000
};

    byte charThermometer0[] = {
        0b00100,
        0b01010,
        0b01010,
        0b01010,
        0b01110,
        0b11111,
        0b11111,
        0b01110
    };

   byte charThermometer1[] = {
        0b00010,
        0b10101,
        0b00101,
        0b10101,
        0b00111,
        0b10111,
        0b00111,
        0b10111

    };
byte charThermometer2[] = {
        0b00010,
        0b11101,
        0b00101,
        0b11101,
        0b00111,
        0b11111,
        0b00111,
        0b11111
    };

byte charThermometer3[] = {
        0b00111,
        0b00101,
        0b00101,
        0b00101,
        0b00111,
        0b00111,
        0b00111,
        0b00111
    };

    byte charThermometer4[] = {
        0b00100,
        0b01010,
        0b01010,
        0b01010,
        0b01010,
        0b10001,
        0b10001,
        0b01110
    };

    byte charThermometer5[] = {
        0b00100,
        0b01010,
        0b01010,
        0b01110,
        0b01110,
        0b11111,
        0b11111,
        0b01110
    };

    byte charThermometer[] = {
        0b00100,
        0b01010,
        0b01010,
        0b01110,
        0b11111,
        0b11111,
        0b01110,
        0b00000
    };

    byte charDegrees[] = {
        0b00100,
        0b01010,
        0b00100,
        0b00000,
        0b00000,
        0b00000,
        0b00000,
        0b00000
    };

    byte charSubscriptTwo[] = {
        0b00000,
        0b00000,
        0b11000,
        0b00100,
        0b11000,
        0b10000,
        0b11100,
        0b00000,
    };

    byte charSuperscriptThree[] = {
        0b00000,
        0b11000,
        0b00100,
        0b11000,
        0b00100,
        0b11000,
        0b00000,
        0b00000,
    };

    byte charWaterDrop0[] = {
        0b00100,
        0b00100,
        0b01010,
        0b10001,
        0b10001,
        0b10001,
        0b01110,
        0b00000
    };

    byte charWaterDrop[] = {
        0b00100,
        0b00100,
        0b01110,
        0b11111,
        0b11101,
        0b11111,
        0b01110,
        0b00000
    };

    byte charMicro[] = {
        0b00000,
        0b00000,
        0b10001,
        0b10001,
        0b11011,
        0b10111,
        0b10000,
        0b00000,
    };

    byte charSmile[] = {
        0b00000,
        0b00000,
        0b01010,
        0b00000,
        0b10001,
        0b01110,
        0b00000,
        0b00000,
    };

    byte charMeh[] = {
        0b00000,
        0b00000,
        0b01010,
        0b00000,
        0b11111,
        0b00000,
        0b00000,
        0b00000,
    };

    byte charSad[] = {
        0b00000,
        0b00000,
        0b01010,
        0b00000,
        0b01110,
        0b10001,
        0b00000,
        0b00000,
    };

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    initLogging(LOG_LEVEL_VERBOSE);

    Log.infoln("eRora Sense");

    std::ostringstream os;
    os << "Built: " << BUILD_DATE << " " << BUILD_TIME;
    Log.infoln(os.str().c_str());

# if 1
    lcd.setup();
    lcd.writeLine(0, "Line 1");
    lcd.writeLine(1, "ABCDEFGHIJKLMNOPQSTUVWXYZ");
    // lcd.writeLine(1, "A very long line that should scroll Line 2.2.2");
#endif

#if 0

    Log.verboseln("At (A)");
    _lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
    _lcd.createChar(0, charSmile);
    _lcd.createChar(1, charFanBlades);   
    _lcd.createChar(2, charThermometer);   
    _lcd.createChar(3, charDegrees);   
    _lcd.createChar(4, charSubscriptTwo);   
    _lcd.createChar(5, charSuperscriptThree);   
    _lcd.createChar(6, charWaterDrop);   
    _lcd.createChar(7, charSad);

  // _lcd.begin(16, 2);
    _lcd.backlight();

    _lcd.clear();
    _lcd.setCursor(2, 0);
    _lcd.print("eRora Sense");
    _lcd.setCursor(0, 1);
    _lcd.print(" TVOC PM CO2 H2 ");

    delay(3000);

    _lcd.clear();
    _lcd.setCursor(2, 0);
    _lcd.print("eRora Sense");
    _lcd.setCursor(0, 1);
    _lcd.print("PWM Fan Control ");

#if 0

    delay(3000);

    _lcd.clear();
    for (int i = 0; i < 8; ++i) {
        _lcd.setCursor(i, 0);
        switch (i) {
            case 0: 
                _lcd.createChar(0, charSad);
                break;
            case 1: 
                _lcd.createChar(0, charMeh);
                break;
            case 2: 
                _lcd.createChar(0, charFanBlades);
                break;
      }
      delay(50);
      _lcd.write(0);
    }

#endif

    /*
    _lcd.setCursor(2,1);
    _lcd.print("Ywrobot Arduino!");
    _lcd.setCursor(0,2);
    _lcd.print("Arduino LCM IIC 2004");
    _lcd.setCursor(2,3);
    _lcd.print("Power By Ec-yuan!");
    */
    Log.verboseln("At (C)");
#endif

    /// scanner.setup();

#if 0

    wifi_setup();

    bmeSensor.setup();
    sgpSensor.setup();
    // personSensor.setup();
    pmSensor.setup();

    envMonitor.setup();
    // envLogger.setup();

    bool reportInitial(true);

    envMonitor.addTemperatureObserver(envLogger, reportInitial);
    envMonitor.addHumidityObserver(envLogger, reportInitial);
    envMonitor.addTVOCObserver(envLogger, reportInitial);
    envMonitor.addCO2Observer(envLogger, reportInitial);
    envMonitor.addHydrogenObserver(envLogger, reportInitial);
    envMonitor.addEthenolObserver(envLogger, reportInitial);
    envMonitor.addParticleObserver(envLogger, reportInitial);

    envMonitor.attachTemperatureSensor(bmeSensor);
    envMonitor.attachHumiditySensor(bmeSensor);
    envMonitor.attachTVOCSensor(sgpSensor);
    envMonitor.attachCO2Sensor(sgpSensor);
    envMonitor.attachHydrogenSensor(sgpSensor);
    envMonitor.attachEthenolSensor(sgpSensor);
    envMonitor.attachParticleSensor(pmSensor);

    #endif

}

CountdownTimer timer(TimeSpan::fromMilliseconds(15000).millis(), CountdownTimer::State::Running);
static int loops(0);

const char* _charMicro() {
    return "\xE4";
}

std::string _makeTopLine() {
    std::stringstream ss;
    ss << "\x02" << "23.7" << "\x03" << "C " << "\x06" << "43%" << " " << "\x01" "10";
    return ss.str();
}

std::string _makeCO2Line() {
    std::stringstream ss;
    // ss << "CO\x04" << " " << "3218" << " \07" << "g/m\05 "; // << (char)0; // " \x00";
    ss << "CO\x04" << " " << "3218" << _charMicro() << "g/m\05  "; // << (char)0; // " \x00";
    std::string str(ss.str());
    //str.push_back(0); // Smiley.  :)
    return str;
}

std::string _makeH2Line()
{
    std::stringstream ss;
    // ss << "H\x04" << " " << "1305" << " \07" << "g/m\05 "; //  << (char)0;// " \x00";
    ss << "H\x04" << " " << "1305" << " \07" << "g/m\05 "; //  << (char)0;// " \x00";
    std::string str(ss.str());
   //str.push_back(0); // Smiley.  :)
    return str;
}

std::string _makeTVOCLine()
{
    std::stringstream ss;
    // ss << "H\x04" << " " << "1305" << " ppm "; // << (char)0; // " \x00";
    ss << "TVOC" << " " << "41305" << "ppm  "; // << (char)0; // " \x00";
    std::string str(ss.str());
    //str.push_back(0); // Smiley.  :)
    return str;
}

std::string _makePM1Line()
{
    std::stringstream ss;
    // ss << "H\x04" << " " << "1305" << " ppm "; // << (char)0; // " \x00";
    ss << "H\x04" << " " << "11305" << "ppm    "; // << (char)0; // " \x00";
    std::string str(ss.str());
    //str.push_back(0); // Smiley.  :)
    return str;
}

uint8_t code = 0;
uint8_t page = 0;

void loop()
{
    /// scanner.loop();

    std::string line2;

    {
        std::ostringstream ss;
        ss << "Loops: " << loops;

        line2 = ss.str();
    }

#if 0
    if (timer.hasExpired()) {
        Log.verboseln("Loops: %d", loops);
        timer.restart();

        lcd.writeLine(0, "Hello World!");
        lcd.writeLine(1, line2);
        lcd.show();
    }
#endif

    lcd.show();   // DOES THS CRASH IT ????!!!? (Maybe fixed --> ctor() resize!)

#if 0
    if (timer.hasExpired())
    {
        Log.verboseln("Loops: %d", loops);
        timer.restart();

        //_lcd.clear();
        _lcd.setCursor(0, 0);
        //_lcd.print("\x02" "23.7" "\x03" "C " "\x06" "43%" "  " "\x01" "7");
        _lcd.print(_makeTopLine().c_str());
        _lcd.setCursor(0, 1);
        if (page == 0) {
          _lcd.print(_makeTVOCLine().c_str());
        } else {
          _lcd.print(_makeCO2Line().c_str());
        }
        //_lcd.createChar(0, charSmile); Uncomment and the smiley fails to be rendered!!!!!!
        _lcd.write(0); 
        //_lcd.createChar(0, charMeh);
        //_lcd.write(0); 
        //_lcd.createChar(0, charSad);
        //_lcd.write(0); 
        // _lcd.print(line2.c_str());

        ++loops;
        page = 1 - page;
    }
#endif

#if 0
    if (timer.hasExpired())
    {
        _lcd.setCursor(0, 0);
        // _lcd.print(_makeTopLine().c_str());
        _lcd.print((int)code);
        _lcd.print("-");
        _lcd.print((int)(code + 15));
        for (int i = 0; i < 16; ++i) {
          _lcd.setCursor(i, 1);
          _lcd.write(code++); 
          // _lcd.write(i); 
        }

        timer.restart();
    }
#endif

    // delay(100);
    
    #if 0

    if (timer.hasExpired()) {
      Log.verboseln("Looped %d times...", loops);
      timer.restart();
    }

    #endif

  // TODO
  #if 0
    sgpSensor.setTemperateAndHumidity(
        bmeSensor.readTemperature(),
        _calculateAbsoluteHumidity(bmeSensor.readTemperature(), bmeSensor.readHumidity()
    );
    #endif

    #if 0

    bmeSensor.loop();

    sgpSensor.loop();
    pmSensor.loop();
    // personSensor.loop();

    envMonitor.loop();

    #endif
}

#endif
