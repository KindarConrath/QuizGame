#include <Arduino.h>
#include <Eventually.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

enum Direction {
  FORWARD = 1,
  BACKWARD = -1
};

enum GameState {
  Startup,
  Playing,
  Answering,
  Waiting
};

enum LightSpeed {
  SLOW = 500,
  MEDIUM = 250,
  FAST = 100 
};

struct Player {
  unsigned char button;
  unsigned char light;
  unsigned char sound;
};

//setup
void setupPins();
void enablePlayerButtons();
void enableControlButtons();

//buttonpress events
void onPlayerPress(EvtListener *lstn);
void onControlButton1();
void onControlButton2();

//random
void stopSounds();
void doTimeout();
bool doIntro();

//lighting controls
void setLights(bool state);

//flashing animation
void flasher(LightSpeed speed, int flashes);
bool blink(int counter);

//state monitor animation
void stateMonitor();

EvtManager mgr;

static const int numPlayers = 6;
static const Player players [numPlayers] = {{A0, 8, 5}, {A1, 9, 6}, {A2, 10, 7}, {A3, 11, 8}, {A4, 12, 9}, {A5, 13, 10}}; //, {A6, 14, 7}, {A7, 15, 8}};
static const int controlButton1 = 1;
static const int controlButton2 = 0;

int timeoutTime = 8000; // time in miliseconds

Player buzzPlayer;

GameState currentState = Startup;

EvtTimeListener* timer;

bool animPlaying = false;

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // connect to pin 2 on the DFPlayer via a 1K resistor
static const uint8_t PIN_MP3_RX = 3; // connect to pin 3 on the DFPlayer

// Software serial library
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Player
DFRobotDFPlayerMini soundPlayer;

void setup() {
  setupPins();
  mgr.addListener(new EvtTimeListener(1, false, (EvtAction)stateMonitor));

  setLights(HIGH);

  enablePlayerButtons();
  enableControlButtons();

  // Init serial port for DFPlayer Mini
  softwareSerial.begin(9600);
  // Start communication with DFPlayer Mini
  if (soundPlayer.begin(softwareSerial)) soundPlayer.volume(30);

  setLights(LOW);

  soundPlayer.play(2); // INTRO SOUND
}

void setupPins() {
    for (int ctr = 8; ctr <= 14; ctr++) {
        pinMode(ctr, OUTPUT);
    }

    for (int ctr = 0; ctr < numPlayers; ctr++) {
      pinMode(players[ctr].button, INPUT_PULLUP);
    }

    pinMode(controlButton1, INPUT_PULLUP);
    pinMode(controlButton2, INPUT_PULLUP);
}

USE_EVENTUALLY_LOOP(mgr)

void enablePlayerButtons() {
  for (int ctr = 0; ctr < numPlayers; ctr++) {
    mgr.addListener(new EvtPinListener(players[ctr].button, 1000, LOW, (EvtAction)onPlayerPress, ctr));
  }
}

void enableControlButtons() {
  mgr.addListener(new EvtPinListener(controlButton1, 500, LOW, (EvtAction)onControlButton1));
  mgr.addListener(new EvtPinListener(controlButton2, 500, LOW, (EvtAction)onControlButton2));
}

void onPlayerPress(EvtListener *lstn) {
  if (currentState == Playing) {
    currentState = Answering;
    Player player = players[(int)(lstn->extraData)];
    digitalWrite(player.light, HIGH);
    soundPlayer.play(player.sound); //PLAYER SPECIFIC DING IN SOUND
    mgr.addListener(new EvtTimeListener(3000, false, (EvtAction)stopSounds));
    timer = new EvtTimeListener(timeoutTime, true, (EvtAction)doTimeout);
    mgr.addListener(timer);
  }
}

void stopSounds() {
  soundPlayer.stop();
}

void onControlButton1() {
  if (currentState == Answering) {
    mgr.removeListener(timer);
    timer=NULL;
    currentState = Waiting;
  } else if (currentState == Waiting) {
    setLights(LOW);
    currentState = Playing;
  } else if (currentState == Playing) {
    doTimeout();
  } else if (currentState == Startup) {
    currentState = Playing;
    soundPlayer.stop();
    mgr.resetContext();
    mgr.addListener(new EvtTimeListener(1, false, (EvtAction)stateMonitor));
    setLights(LOW);
    enablePlayerButtons();
    enableControlButtons();
    flasher(MEDIUM, 4);
  }
}

void onControlButton2() {
  if (currentState == Answering || currentState == Waiting) {
    soundPlayer.play(3); //CORRECT ANSWER SOUND
    if (timer) {
      mgr.removeListener(timer);
      timer = NULL;
    }
    currentState = Playing;
  } else if (currentState == Playing) {
    soundPlayer.play(4); //THINKING MUSIC
  }
}

void doTimeout() {
  setLights(LOW);
  if (currentState == Answering) {
    currentState = Waiting;
    soundPlayer.play(1); //TIMEOUT SOUND
    timer = NULL;
    mgr.addListener(new EvtTimeListener(3000, false, (EvtAction)stopSounds));
  }
}

void setLights(bool state) {
  for(Player player : players) {
    digitalWrite(player.light, state);
  }
}

// FLASHING ANIMATION //
bool blinkState = LOW;
int numFlashes = 0;

void flasher(LightSpeed speed, int flashes) {
  setLights(HIGH);
  numFlashes = flashes * 2;
  blinkState = LOW;
  mgr.addListener(new EvtTimeListener(speed, true, (EvtAction)blink));
}

bool blink(int counter) {
  setLights(blinkState);
  blinkState = !blinkState;

  numFlashes--;
  return numFlashes <= 0;
}

struct anim {
  bool state;
  int time;
};

static const anim startupAnim [] = {{HIGH,200}, {LOW,200}}; //constant flashing
static const anim playingAnim [] = {{HIGH,200}, {LOW,500}, {LOW, 500}}; //quick blink, with pause
static const anim answeringAnim [] = {{HIGH,200}, {LOW,200}, {HIGH,500}, {HIGH,500}, {LOW,200}}; //quick blink, long blink
static const anim waitingAnim [] = {{HIGH,500}}; //constant on

GameState prevState = Startup;
int animCtr = 0;

void stateMonitor() {
  const anim *currentAnim;

  if (currentState != prevState) {
    animCtr = 0;
    prevState = currentState;
  }

  switch(currentState) {
    case Startup:
      currentAnim = startupAnim;
      break;
    case Playing:
      currentAnim = playingAnim;
      break;
    case Answering:
      currentAnim = answeringAnim;
      break;
    case Waiting:
      currentAnim = waitingAnim;
      break;
  }

  int arrSize = sizeof(currentAnim) / sizeof(anim);
  if (animCtr > arrSize) {
    animCtr = 0;
  }
  digitalWrite(14, (currentAnim + animCtr)->state);

  animCtr++;
  
  mgr.addListener(new EvtTimeListener((currentAnim + animCtr)->time , false, (EvtAction)stateMonitor));
}
