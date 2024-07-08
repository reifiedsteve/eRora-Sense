#if 0

#include <FastLED.h>




#if 1

#include "CabinetLights.h"

#include "Drivers/MomentaryButton.h"


CabinetLights cabinet(30);

MomentaryButton powerButton(27);
MomentaryButton inspectionButton(14);

bool power(false);

void setup()
{
    Serial.begin(115200);

    initLogging(LOG_LEVEL_VERBOSE);


    Log.infoln("Starting up.");

    cabinet.setup();

    cabinet.setColour(CRGB(0, 0, 255));
    cabinet.setPower(true);

    cabinet.setInspectionTime(TimeSpan::fromSeconds(5));
    cabinet.setInspectionColour(CRGB(255, 255, 255));

    cabinet.setPower(false);

    powerButton.attachClick([cabinet]() {
        power = !power;
        Log.verboseln("Setting power to %s.", power ? "on" : "off");
        cabinet.setPower(power);
    });

    inspectionButton.attachClick([cabinet]() {
        Log.verboseln("Triggering inspection light.");
        cabinet.triggerInspectionLight();
    });
}   

void loop()
{
    powerButton.tick();
    inspectionButton.tick();

    cabinet.loop();
}

#else

// Number of LEDs
#define DATA_PIN 25
#define NUM_LEDS 30
// LED array
CRGB leds[NUM_LEDS];

void setup() {
    // Initialize the LEDs
    FastLED.addLeds<WS2812B, DATA_PIN>(leds, NUM_LEDS);
    // FastLED.setBrightness(255);
}

void loop() 
{
    // Update LED colors
    CRGB rgb(random8(), random8(), random8());

    for (int i = 0; i < NUM_LEDS; i++) {
        // Set a random color for each LED
        leds[i] = rgb;
    }
  
    // Apply the color changes to the LED strip
    FastLED.show();
  
    // Delay before updating colors again
    delay(500);
}

#endif

#endif
