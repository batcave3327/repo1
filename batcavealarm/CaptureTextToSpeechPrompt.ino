#include <Bridge.h>
#include <Temboo.h>
#include "utility/TembooGPIO.h"
#include "TembooAccount.h" // Contains Temboo account information  
 
// The number of times to trigger the action if the condition is met
// We limit this so you won't use all of your Temboo calls while testing
int maxCalls = 10;

// The number of times this Choreo has been run so far in this sketch
int calls = 0;

// Declaring sensor configs
TembooGPIOConfig sensor2Config;
TembooGPIOConfig actuator4Config;

// Declaring TembooSensors
TembooSensor sensor2;
TembooSensor actuator4;

void setup() {
  Serial.begin(9600);

  // For debugging, wait until the serial console is connected
  delay(4000);
  while(!Serial);
  Bridge.begin();
  
  // Initialize sensors and configs
  tembooDigitalGPIOInit(&sensor2Config, &sensor2, 2, LOW, INPUT);
  tembooDigitalGPIOInit(&actuator4Config, &actuator4, 4, LOW, OUTPUT);

  Serial.println("Setup complete.\n");
}

void loop() {
  int sensorValue = sensor2.read(sensor2.sensorConfig);
  Serial.println("Sensor: " + String(sensorValue));
  if (sensorValue == HIGH) {
    if (calls < maxCalls) {
      Serial.println("\nTriggered! Calling CaptureTextToSpeechPrompt Choreo...");
      runCaptureTextToSpeechPrompt(sensorValue);
      calls++;
    } else {
      Serial.println("\nTriggered! Skipping to save Temboo calls. Adjust maxCalls as required.");
    }
  }
  delay(250);
}

void runCaptureTextToSpeechPrompt(int sensorValue) {
  TembooChoreo CaptureTextToSpeechPromptChoreo;

  // Invoke the Temboo client
  CaptureTextToSpeechPromptChoreo.begin();

  // Set Temboo account credentials
  CaptureTextToSpeechPromptChoreo.setAccountName(TEMBOO_ACCOUNT);
  CaptureTextToSpeechPromptChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  CaptureTextToSpeechPromptChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set profile to use for execution
  CaptureTextToSpeechPromptChoreo.setProfile("BatcaveAlarm");
  // Identify the Choreo to run
  CaptureTextToSpeechPromptChoreo.setChoreo("/Library/Nexmo/Voice/CaptureTextToSpeechPrompt");

  // Run the Choreo
  unsigned int returnCode = CaptureTextToSpeechPromptChoreo.run();

  // A return code of zero means everything worked
  if (returnCode == 0) {
    while (CaptureTextToSpeechPromptChoreo.available()) {
      String name = CaptureTextToSpeechPromptChoreo.readStringUntil('\x1F');
      name.trim();

      String data = CaptureTextToSpeechPromptChoreo.readStringUntil('\x1E');
      data.trim();

      if (name == "Digits") {
        if (data.toInt() == 1) {
          actuator4.write(actuator4.sensorConfig, HIGH);
        }
      }
    }
  }
  CaptureTextToSpeechPromptChoreo.close();
}