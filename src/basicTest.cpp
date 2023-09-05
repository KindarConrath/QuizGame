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
        pinMode(player.button, INPUT);
    }
    pinMode(13, OUTPUT);
}

void loop() {
    if (digitalRead(players[0].button == HIGH)) {
        digitalWrite(13, HIGH);
    } else {
        digitalWrite(13, LOW);
    }
}