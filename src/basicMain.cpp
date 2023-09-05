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

int timeoutTime = 8000; // time in miliseconds

static const int numPlayers = 6;
static const Player players [numPlayers] = {{A0, 8, 1}, {A1, 9, 2}, {A2, 10, 3}, {A3, 11, 4}, {A4, 12, 5}, {A5, 13, 6}}; //, {A6, 14, 7}, {A7, 15, 8}}; 
static const int controlButton1 = 1;
static const int controlButton2 = 0;

GameState currentState = STARTUP;

void onBuzzerPressed(pinid_t, bool);
void cb1Pressed(pinid_t, bool);
void cb2Pressed(pinid_t, bool);
void doTimeout();
void setLights(bool);
int posMod(int a, int b);

taskid_t timeoutTimer = NULL;

void setup() {
    switches.init(asIoRef(internalDigitalDevice()), SWITCHES_POLL_EVERYTHING, true);

    for(Player player : players) {
        internalDigitalDevice().pinMode(player.button, INPUT_PULLUP);
        internalDigitalDevice().pinMode(player.light, OUTPUT);
        switches.addSwitch(player.button, onBuzzerPressed, NO_REPEAT);
    }

    switches.addSwitch(controlButton1, cb1Pressed, NO_REPEAT);
    switches.addSwitch(controlButton2, cb2Pressed, NO_REPEAT);
}

void loop() {
    taskManager.runLoop();
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
        timeoutTimer = taskManager.scheduleOnce(timeoutTime, doTimeout);
    }
}

void cb1Pressed(pinid_t pin, bool healdDown) {
  if (currentState == ANSWERING) {
    taskManager.cancelTask(timeoutTimer);
    timeoutTimer = NULL;
    currentState = WAITING;
  } else if (currentState == WAITING) {
    setLights(LOW);
    currentState = PLAYING;
  } else if (currentState == PLAYING) {
    doTimeout();
  } else if (currentState == STARTUP) {
    currentState = PLAYING;
    setLights(LOW);
  }
}

void cb2Pressed(pinid_t pin, bool healdDown) {
  if (currentState == ANSWERING || currentState == WAITING) {
    if (timeoutTimer) {
      taskManager.cancelTask(timeoutTimer);
      timeoutTimer = NULL;
    }
    currentState = PLAYING;
  }
}

struct anim {
  bool state;
  int time;
};

void doTimeout() {
  setLights(LOW);
  if (currentState == ANSWERING) {
    currentState = WAITING;
    timeoutTimer = NULL;
  }
}

void setLights(bool state) {
  for(Player player : players) {
    internalDigitalDevice().digitalWriteS(player.light, state);
  }
}
