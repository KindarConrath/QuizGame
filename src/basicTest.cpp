#include <Arduino.h>

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

void setup() {
    for (Player player : players) {
        pinMode(player.button, INPUT_PULLUP);
        pinMode(player.light, OUTPUT);
    }

    pinMode(controlButton1, INPUT_PULLUP);
    pinMode(controlButton2, INPUT_PULLUP);
    pinMode(controlLight, OUTPUT);
}

void loop() {
    if (digitalRead(controlButton1) == LOW) {
        doLoop = false;
    } 
    
    if (digitalRead(controlButton2) == LOW) {
        doLoop = true;
    }

    if (doLoop) {
        digitalWrite(controlLight, state);
        for (Player player : players) {
            digitalWrite(player.light, state);
            state = !state;
        }
    }

    if (!doLoop) {
        for (Player player : players) {
            digitalWrite(player.light, LOW);
            if (digitalRead(player.button) == LOW) {
                digitalWrite(player.light, HIGH);
            }
        }
    }
}