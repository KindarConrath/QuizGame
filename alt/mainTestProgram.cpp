#include <Arduino.h>
#include <string.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

struct Player {
  unsigned char button;
  unsigned char light;
  unsigned char sound;
};

int timeoutTime = 8000; // time in miliseconds

static const int numPlayers = 6;
static const Player players [numPlayers] = {{A0, 8, 1}, {A1, 9, 2}, {A2, 10, 3}, {A3, 11, 4}, {A4, 12, 5}, {A5, 13, 6}};

static const int controlButton1 = 1;
static const int controlButton2 = 0;

static const int controlLight = 7;

bool doLoop = true;
bool state = HIGH;

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // connect to pin 2 on the DFPlayer via a 1K resistor
static const uint8_t PIN_MP3_RX = 3; // connect to pin 3 on the DFPlayer

// Software serial library
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Player
DFRobotDFPlayerMini soundPlayer;

void setup() {
    Serial.begin(9600);
    for (Player player : players) {
        pinMode(player.button, INPUT_PULLUP);
        pinMode(player.light, OUTPUT);
    }

    // Init serial port for DFPlayer Mini
    softwareSerial.begin(9600);
    // Start communication with DFPlayer Mini
    if (soundPlayer.begin(softwareSerial)) soundPlayer.volume(30);
    
    pinMode(13, OUTPUT);
    pinMode(0, INPUT_PULLUP);
    pinMode(1, INPUT_PULLUP);
}

void loop() {
    // for (Player player : players) {
    //     Serial.println("Player: " + String(player.sound) + " - " + (digitalRead(player.button) ? "HIGH" : "LOW"));
    // }

    soundPlayer.play(1);
    delay(500);
    bool lightTriggered = false;
    if(digitalRead(0) == LOW || digitalRead(1) == LOW) {
        lightTriggered = true;
    }

    Serial.println("CB1: " + String(digitalRead(0)));
    Serial.println("CB2: " + String(digitalRead(1)));

    for (Player player : players) {
        if (digitalRead(player.button) == LOW) {
            Serial.println("Player: " + String(player.sound) + " PRESSED");
            digitalWrite(player.light, HIGH);
            lightTriggered = true;
        } else {
            digitalWrite(player.light, LOW);
        }
    }
    digitalWrite(13, lightTriggered);
}