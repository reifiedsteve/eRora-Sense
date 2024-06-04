#if 0

#include <Arduino.h>

#include "Fan/PWMFanController.h"
#include "Chronos/CountdownTimer.h"
#include "Diagnostics/Logging.h"

PWMFanController fan(36);
bool fast(false);
int fastSpeed(100);
int slowSpeed(20);

CountdownTimer timer(10000);

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Let's begin. :)");

    initLogging(LOG_LEVEL_VERBOSE);

    fan.setup();
    fan.setFanSpeed(slowSpeed);
    timer.restart();
}


void loop()
{
    if (timer.hasExpired()) {
        fast = !fast;
        fan.setFanSpeed(fast ? fastSpeed : slowSpeed);
        Serial.print("Fan speed changed to ");
        Serial.print(fast ? "fast" : "slow");
        Serial.println();
        timer.restart();
    }
}

#endif