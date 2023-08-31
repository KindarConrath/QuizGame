#include "DFRobotDFPlayerMini.h"
#include "SoftwareSerial.h"

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // connect to pin 2 on the DFPlayer via a 1K resistor
static const uint8_t PIN_MP3_RX = 3; // connect to pin 3 on the DFPlayer

// Software serial library
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Player
DFRobotDFPlayerMini player;

// Pins for buttons
const unsigned char switches[4] = {A0, A1, A2, A3};

// Pins for LEDs
const unsigned char leds[4] = {9, 10, 11, 12};

// Reset button
const unsigned char switch_reset = 6;

// Prepare the Arduino
void setup() {
  // Prepare the pins
  pinMode(switch_reset, INPUT_PULLUP);  
  for (unsigned char sw=0; sw<4; sw++) {
    pinMode(switches[sw], INPUT_PULLUP);
    pinMode(leds[sw], OUTPUT);
    digitalWrite(leds[sw], LOW);
  }
  
  // Init serial port for DFPlayer Mini
  softwareSerial.begin(9600);
  // Start communication with DFPlayer Mini
  if (player.begin(softwareSerial)) player.volume(30);
}

// Main loop
void loop() {
  // Check all 4 switches
  for (unsigned char sw=0; sw<4; sw++) {
    // Is it pressed?
    if (digitalRead(switches[sw]) == LOW) {
      // Turn on its LED
      digitalWrite(leds[sw], HIGH);
      // Play its sfx
      player.play(sw+1);
      // Wait for the reset button
      while (digitalRead(switch_reset) == HIGH) {};
      // Stop any playing sfx
      player.stop();
      // Turn off the LED
      digitalWrite(leds[sw], LOW);
      // Stop!
      break;
    }
  }
  if (digitalRead(switch_reset) == LOW) {
    player.play(9);
    delay(3000);
    player.stop();
  }
}
