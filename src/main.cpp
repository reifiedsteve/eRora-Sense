#include <SPIFFS.h>

#include "MyEroraSensor.h"
#include "Diagnostics/Logging.h"

#include "PersistentSettings.h"

#include "Chronos/Stopwatch.h"

#include "UserSettings.h"

#include "Diagnostics/OnboardLEDBlinker.h"

#define DIAGNOSE_RESTART_REASON
#define DO_I2C_SCAN

#ifdef DO_I2C_SCAN
#include "Diagnostics/ScannerI2C.h"
ScannerI2C scanner;
#endif

#include "MyEroraSensor.h"

MyEroraSensor* myErora;

//CharacterMatrix1602 lcd;
// LD2410Sensor personSensor(27);
// PMS7003ParticleSensor pmSensor(16, 17, TimeSpan::fromSeconds(5)); // TODO: make say 10 minutes (to avoid sensor wearing out).

#ifdef DIAGNOSE_RESTART_REASON
OnboardLEDBlinker blinker;
#endif

void initFileSystem()
{
    if(!SPIFFS.begin(false)) { // NOTE: needs to be true on first run in order to format the FS (but thereafter don't want it to attempt a reformat simply because of a mount failure).
        Log.errorln("An Error has occurred while mounting SPIFFS.");
        return;        
    }

    // PersistentSettings::listAll(false);

    Log.infoln("FileAccess: file system has used %d out of a total of %d bytes.", SPIFFS.usedBytes(), SPIFFS.totalBytes());
}

unsigned getRestartReasonBlinkCode()
{
    esp_reset_reason_t reason(esp_reset_reason());
    unsigned blinks = 0;

    switch (reason)
    {
        case ESP_RST_UNKNOWN:    //!< Reset reason can not be determined
            blinks = 0;
            break;
        case ESP_RST_POWERON:    //!< Reset due to power-on event
            blinks = 1;
            break;
        case ESP_RST_EXT:        //!< Reset by external pin (not applicable for ESP32)
            blinks = 2;
            break;
        case ESP_RST_SW:         //!< Software reset via esp_restart
            blinks = 3;
            break;
        case ESP_RST_PANIC:      //!< Software reset due to exception/panic
            blinks = 4;
            break;
        case ESP_RST_INT_WDT:    //!< Reset (software or hardware) due to interrupt watchdog
            blinks = 5;
            break;
        case ESP_RST_TASK_WDT:   //!< Reset due to task watchdog
            blinks = 6;
            break;
        case ESP_RST_WDT:        //!< Reset due to other watchdogs
            blinks = 7;
            break;
        case ESP_RST_DEEPSLEEP:  //!< Reset after exiting deep sleep mode
            blinks = 8;
            break;
        case ESP_RST_BROWNOUT:   //!< Brownout reset (software or hardware)
            blinks = 9;
            break;
        case ESP_RST_SDIO:       //!< Reset over SDIO
            blinks = 10;
            break;
    }

    return blinks;
}

void setup() 
{
    Serial.begin(115200);

    initLogging(LOG_LEVEL_VERBOSE);
    /// initLogging(LOG_LEVEL_INFO);

    Log.infoln("============================================");
    Log.infoln("eRora smart multi-sensor. ver 0.1a");
    Log.infoln("Built on %s at %s.", __DATE__, __TIME__);
    Log.infoln("Author: Steve Morley.");
    Log.infoln("Web: www.reified.co.uk");
    Log.infoln("(c) 2024 Reified Ltd.");
    Log.infoln("============================================");

    #ifdef DISABLE_LOGGING
        std::cout << "WARNING: all logging is disabled!" << std::endl;
    #endif

    initFileSystem();

    myErora = new MyEroraSensor(blinker);
    myErora->setup();

  // unsigned numBlinks = (unsigned)esp_reset_reason();

    #ifdef DIAGNOSE_RESTART_REASON
        int restartReasonCode(getRestartReasonBlinkCode());
        Log.infoln("main: restart reason code is %d.", restartReasonCode);
        blinker.start(restartReasonCode, 500, 500);
    #endif
}

void loop() {
    myErora->loop();
    #ifdef DIAGNOSE_RESTART_REASON
        blinker.loop();
    #endif
}
