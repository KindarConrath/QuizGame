#include <IoAbstraction.h>
#include <TaskManagerIO.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

enum GameState {
  STARTUP,
  PLAYING,
  ANSWERING,
  WAITING
};

struct Player {
  unsigned char button;
  unsigned char light;
  unsigned char sound;
};

static const int numPlayers = 6;
static const Player players [numPlayers] = {{A0, 5, 1}, {A1, 6, 2}, {A2, 7, 3}, {A3, 8, 4}, {A4, 9, 5}, {A5, 10, 6}}; //, {A6, 14, 7}, {A7, 15, 8}}; 
static const int controlButton1 = 1;
static const int controlButton2 = 0;

GameState currentState = STARTUP;

void onBuzzerPressed(pinid_t, bool);
void cb1Pressed(pinid_t, bool);
void cb2Pressed(pinid_t, bool);
void setLights(bool);
void flashLights();

void setup() {
    switches.init(asIoRef(internalDigitalDevice()), SWITCHES_POLL_EVERYTHING, true);

    for(Player player : players) {
        internalDigitalDevice().pinMode(player.button, INPUT_PULLUP);
        internalDigitalDevice().pinMode(player.light, OUTPUT);
        switches.addSwitch(player.button, onBuzzerPressed, NO_REPEAT);
    }

    switches.addSwitch(controlButton1, cb1Pressed, NO_REPEAT);
    switches.addSwitch(controlButton2, cb2Pressed, NO_REPEAT);

    flashLights();
    currentState = PLAYING;
}

void loop() {
    taskManager.runLoop();
}

void flashLights() {
  setLights(HIGH);
  delay(500);
  setLights(LOW);
  delay(250);
  setLights(HIGH);
  delay(500);
  setLights(LOW);
}

void onBuzzerPressed(pinid_t pin, bool healdDown) {
    if (currentState == PLAYING) {
        currentState = ANSWERING;
        Player currentPlayer;
        for(Player player : players) {
            if (player.button == pin) {
                currentPlayer = player;
                break;
            }
        }

        internalDigitalDevice().digitalWriteS(currentPlayer.light, HIGH);
    }
}

void cb1Pressed(pinid_t pin, bool healdDown) {
  if (currentState == ANSWERING) {
    currentState = PLAYING;
    flashLights();
  }
}

void cb2Pressed(pinid_t pin, bool healdDown) {
  if (currentState == ANSWERING) {
    currentState = PLAYING;
    flashLights();
  }
}

void setLights(bool state) {
  for(Player player : players) {
    internalDigitalDevice().digitalWriteS(player.light, state);
  }
}
