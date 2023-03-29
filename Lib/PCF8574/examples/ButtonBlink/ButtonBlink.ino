/**
 * This demo code demonstrate all functionnalities of the PCF8574 library.
 *
 * PCF8574 pins map :
 * 0: led
 * 1: led
 * 2: led
 * 3: button + pull-up resistor
 */

/* Dependencies */
#include <Wire.h>    // Required for I2C communication
#include "PCF8574.h" // Required for PCF8574

/** PCF8574 instance */
PCF8574 expander;

/** setup() */
void setup() {

  /* Setup serial for debug */
  Serial.begin(115200);
  
  /* Start I2C bus and PCF8574 instance */
  expander.begin(0x38);
  
  /* Setup some PCF8574 pins for demo */
  expander.pinMode(0, OUTPUT);
  expander.pinMode(1, OUTPUT);
  expander.pinMode(2, OUTPUT);
 
  
}
void loop() {

  /* DigitalWrite demo */
  expander.digitalWrite(1, HIGH); // Turn off led 2
  delay(500);
  expander.digitalWrite(2, HIGH); // Turn off led 3
  delay(500);
  expander.digitalWrite(3, LOW);  // Turn on led 2
  delay(500);
  
  delay(5000);
}
