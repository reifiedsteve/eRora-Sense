#if 0

#include <WiFI.h>
#include <WiFiClient.h>
#include "MQTTConnection.h"
#include "Diagnostics/Logging.h"

WiFiClient _wiFiClient;
MQTTConnection* _mqtt;

typedef MQTTConnection::Topic Topic;

#include "Chronos/CountdownTimer.h"
CountdownTimer _timer(5000);
int n = 0;

//const char* ssid = "133381";
//const char* password = "Spongebob2000";
const char* ssid = "133381B";
const char* password = "spongebob2000";

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    //Log.verboseln("At (0).");
    //WiFi.begin(ssid, password);

    //while (WiFi.status() != WL_CONNECTED) {
    //    delay(500);
    //    Serial.print(".");
    //}

    //Log.setLevel(LOG_LEVEL_VERBOSE);
    //Log.setShowLevel(LOG_LEVEL_VERBOSE);

    //Log.verboseln("At (A).");
    Serial.print("Flash size: ");
    Serial.println(ESP.getFlashChipSize());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());    

    //Log.verboseln("At (B).");
    
    //_mqtt = new MQTTConnection(_wiFiClient, "192.168.1.186", 1883);

    //Log.verboseln("At (C).");

    //_mqtt->setTopicPrefix(Topic("eRora/test"));
    //_mqtt->setup();
    //_mqtt->publish(Topic("counter"), (long)n);

    _timer.start();

    //Log.verboseln("Setup complete.");

}

void loop(void)
{
    if (_timer.hasExpired()) {
        ++n;
        Log.verboseln("Publishing: %d", n);
        Serial.println("Count: ");
        Serial.println(n);    
        //_mqtt->publish(Topic("counter"), (long)n);
        _timer.restart();
    }

    else {
        //Log.verboseln("Hi");
    }
}

#endif

#if 0

#include "bsec.h"

#define LED_BUILTIN 2

// Helper functions declarations
void myCheckIaqSensorStatus(void);
void checkIaqSensorStatus(void);
void errLeds(void);

// Create an object of the class Bsec
Bsec iaqSensor;

String output;

// Entry point for the example
void setup(void)
{
  /* Initializes the Serial communication */
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
  iaqSensor.begin(BME68X_I2C_ADDR_LOW, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[13] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_GAS_PERCENTAGE
  };

  iaqSensor.updateSubscription(sensorList, 13, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();

  // Print the header
  output = "Timestamp [ms], IAQ, IAQ accuracy, Static IAQ, CO2 equivalent, breath VOC equivalent, raw temp[°C], pressure [hPa], raw relative humidity [%], gas [Ohm], Stab Status, run in status, comp temp[°C], comp humidity [%], gas percentage";
  Serial.println(output);
}

// Function that is looped forever
void loop(void)
{
  unsigned long time_trigger = millis();
  if (iaqSensor.run()) { // If new data is available
    digitalWrite(LED_BUILTIN, LOW);
    output = String(time_trigger);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.stabStatus);
    output += ", " + String(iaqSensor.runInStatus);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.gasPercentage);
    Serial.println(output);
    digitalWrite(LED_BUILTIN, HIGH);
  } 
  
  else {
    myCheckIaqSensorStatus();
  }
}

bsec_library_return_t gBsecError(BSEC_OK);
int8_t gBme68xError(BME68X_OK);

void myCheckIaqSensorStatus(void)
{
    bsec_library_return_t bsecStatus = iaqSensor.bsecStatus;

    if (bsecStatus != gBsecError) {
        Serial.println("BSEC status now: " + String((int)bsecStatus) + " (from " + String((int)gBsecError) + ")");
        gBsecError = bsecStatus;
    }

    int8_t bme68xStatus = iaqSensor.bme68xStatus;

    if (bme68xStatus != gBme68xError) {
        Serial.println("BME68x status now: " + String((int)bme68xStatus) + " (from " + String((int)gBme68xError) + ")");
        gBme68xError = bme68xStatus;
    }
}

// Helper function definitions
void checkIaqSensorStatus(void)
{
    bsec_library_return_t bsecError = iaqSensor.bsecStatus;

  if (bsecError != BSEC_OK) {
    if (bsecError < BSEC_OK) {
      output = "BSEC error code : " + String((int)bsecError);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BSEC warning code : " + String((int)bsecError);
      Serial.println(output);
    }
  }

    uint8_t bme68xError = iaqSensor.bme68xStatus;

  if (bme68xError != BME68X_OK) {
    if (bme68xError < BME68X_OK) {
      output = "BME68X error code : " + String(bme68xError);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    } else {
      output = "BME68X warning code : " + String(bme68xError);
      Serial.println(output);
    }
  }
}

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

#endif
