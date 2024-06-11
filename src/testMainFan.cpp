#if 0

#include <Arduino.h>

#include "Fan/PWMFanController.h"
#include "Chronos/CountdownTimer.h"
#include "Diagnostics/Logging.h"
#include "PinAssignments.h"

bool fast(false);
int fastSpeed(100);
int mediumSpeed(5);
int slowSpeed(25);
uint32_t freq(25000);
uint8_t resolution(8);
uint8_t channelNo(1);

PWMFanController fan(PinAssignments::FanPWM, freq, resolution, channelNo);

CountdownTimer timer(20000);

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Let's begin. :)");

    initLogging(LOG_LEVEL_VERBOSE);

    // fan.configSeparatePowerControlPin(xxx);
    fan.limitPhysicalSpeedRange(50, 100);
    fan.begin();
    fan.setPower(true);
    fan.setDutyCycle(slowSpeed);
    timer.restart();
}


void loop()
{
    if (timer.hasExpired()) {
        fast = !fast;
        #if 1
            Serial.print("Fan switched ");
            Serial.print(fast ? "on" : "off");
            Serial.println();
            fan.setPower(fast);
        #else
            Serial.print("Fan speed changed to ");
            Serial.print(fast ? "fast" : "slow");
            Serial.println();
            fan.setFanSpeed(fast ? fastSpeed : slowSpeed);
        #endif
        timer.restart();
    }
}

#endif