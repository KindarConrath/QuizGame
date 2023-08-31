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
void doIntro();

//lighting controls
void setLights(bool state);
void lightsOn();
void lightsOff();

//off runner animation
void offRunner(LightSpeed speed, int runs, Direction direction);
void startOffRunIntro();
bool offRunIntro();
void startOffRun();
bool offRun();
void startOffRunOutro();
bool offRunOutro();

//on runner animation
void onRunner(LightSpeed speed, int numRuns, bool bounce, Direction direction);
bool onRunNext();

//two way runner animation
void twoWayRunner(LightSpeed speed, int numRuns);
bool twoWayLoop();

//flashing animation
void flasher(LightSpeed speed, int flashes);
bool blink(int counter);

//state monitor animation
void stateMonitor();

EvtManager mgr;

static const int numPlayers = 6;
static const Player players [numPlayers] = {{A0, 8, 1}, {A1, 9, 2}, {A2, 10, 3}, {A3, 11, 4}, {A4, 12, 5}, {A5, 13, 6}}; //, {A6, 14, 7}, {A7, 15, 8}};
static const int controlButton1 = 1;
static const int controlButton2 = 0;

int timeoutTime = 8000; // time in miliseconds

Player buzzPlayer;

GameState currentState = Startup;

EvtTimeListener* timer;

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

  lightsOn();

  enablePlayerButtons();
  enableControlButtons();

  // Init serial port for DFPlayer Mini
  softwareSerial.begin(9600);
  // Start communication with DFPlayer Mini
  if (soundPlayer.begin(softwareSerial)) soundPlayer.volume(30);

  lightsOff();

  doIntro();
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
    lightsOff();
    currentState = Playing;
  } else if (currentState == Playing) {
    doTimeout();
  } else if (currentState == Startup) {
    currentState = Playing;
    soundPlayer.stop();
    mgr.resetContext();
    mgr.addListener(new EvtTimeListener(1, false, (EvtAction)stateMonitor));
    lightsOff();
    enablePlayerButtons();
    enableControlButtons();
    flasher(MEDIUM, 4);
  }
}

void onControlButton2() {
  if (currentState == Answering || currentState == Waiting) {
    soundPlayer.play(14); //CORRECT ANSWER SOUND
    if (timer) {
      mgr.removeListener(timer);
      timer = NULL;
    }
    currentState = Playing;
  } else if (currentState == Playing) {
    soundPlayer.play(15); //THINKING MUSIC
  }
}

void doTimeout() {
  lightsOff();
  if (currentState == Answering) {
    currentState = Waiting;
    soundPlayer.play(9); //TIMEOUT SOUND
    timer = NULL;
    mgr.addListener(new EvtTimeListener(3000, false, (EvtAction)stopSounds));
  }
}

void setLights(bool state) {
  for(Player player : players) {
    digitalWrite(player.light, state);
  }
}

void lightsOn() {
  setLights(HIGH);
}

void lightsOff() {
  setLights(LOW);
}

// ONE LIGHT OFF RUNNING ANIMATION //
int offRunAnimSpeed = MEDIUM;
int numOffRuns = 0;
int offRunDirection = FORWARD;
void offRunner(LightSpeed speed, int runs, Direction direction) {
  offRunAnimSpeed = speed;
  numOffRuns = runs;
  offRunDirection = direction;

  mgr.addListener(new EvtTimeListener(offRunAnimSpeed, false, (EvtAction)startOffRunIntro));
  mgr.addListener(new EvtTimeListener(offRunAnimSpeed * numPlayers, false, (EvtAction)startOffRun));
  mgr.addListener(new EvtTimeListener(offRunAnimSpeed * numPlayers + offRunAnimSpeed * numPlayers * numOffRuns, false, (EvtAction)startOffRunOutro));
}

int offRunCtr = 0;
void startOffRunIntro() {
  offRunCtr = offRunDirection == FORWARD ? 0 : numPlayers - 1;
  mgr.addListener(new EvtTimeListener(offRunAnimSpeed, true, (EvtAction)offRunIntro));  
}

bool offRunIntro() {
  digitalWrite(players[offRunCtr].light, HIGH);
  offRunCtr+=offRunDirection;
  return offRunCtr>=numPlayers || offRunCtr < 0;
}

void startOffRun() {
  offRunCtr = offRunDirection == FORWARD ? 0 : numPlayers - 1;
  mgr.addListener(new EvtTimeListener(offRunAnimSpeed, true, (EvtAction)offRun));  
}

bool offRun() {
  digitalWrite(players[offRunCtr].light, LOW);
  digitalWrite(players[posMod((offRunCtr-offRunDirection),numPlayers)].light,HIGH);
  offRunCtr+=offRunDirection;
  return offRunCtr >= numOffRuns || offRunCtr < 0;
}

void startOffRunOutro() {
  offRunCtr = offRunDirection == FORWARD ? 0 : numPlayers - 1;
  mgr.addListener(new EvtTimeListener(offRunAnimSpeed, true, (EvtAction)offRunOutro));    
}

bool offRunOutro() {
  digitalWrite(players[offRunCtr].light, LOW);
  offRunCtr+=offRunDirection;
  return offRunCtr>=numPlayers || offRunCtr < 0;
}

int posMod(int a, int b) {
  return (b + (a % b)) % b
}

// void blockingRunnerIntro(LightSpeed speed) {
//   for(Player player : players) {
//     delay(speed);
//     digitalWrite(player.light, HIGH);
//   }
// }

// void blockingRunnerOutro(LightSpeed speed) {
//   for (Player player : players) {
//     delay(speed);
//     digitalWrite(player.light, LOW);
//   }
// }

// void blockingOffRunner(LightSpeed speed, int numRuns) {
//   blockingRunnerIntro(speed);

//   for(int numLoops = 0; numLoops < numRuns; numLoops++) {
//     for(Player player : players) {
//       digitalWrite(player.light, LOW);
//       delay(speed);
//       digitalWrite(player.light, HIGH);
//     }
//   }

//   blockingRunnerOutro(speed);
// }

// ONE LIGHT ON RUNNING ANIMATION //
int numLoops = 0;
int runDirection = FORWARD;
bool animBounce = false;
int runPosition = 0;
LightSpeed runSpeed = SLOW;

void onRunner(LightSpeed speed, int numRuns, bool bounce, Direction direction = FORWARD) {
  lightsOff();
  runDirection = direction;
  runPosition = runDirection == FORWARD ? 0 : numPlayers - 1;
  animBounce = bounce;
  numLoops = animBounce ? numRuns * 2 : numRuns;
  runSpeed = speed;
  mgr.addListener(new EvtTimeListener(runSpeed, true, (EvtAction)onRunNext));
}

bool onRunNext() {
  digitalWrite(players[posMod((runPosition - runDirection), numPlayers)].light, LOW);
  digitalWrite(players[runPosition % numPlayers].light, HIGH);

  runPosition+=runDirection;
  if (runPosition < 0 || runPosition >= numPlayers) {
    if (animBounce) {
      runDirection*=-1;
    }
    runPosition = runDirection == FORWARD ? 0 : numPlayers - 1;
    return true;
  }
  return false;
}

// void blockingOnRunner(LightSpeed speed, int numRuns, bool bounce, int direction = FORWARD) {
//     lightsOff();
//     if (bounce) {
//       numRuns*=2; //double number of runs
//     }

//     for(int runs = 0; runs < numRuns; runs++) {
//       for(int player = 0; player < numPlayers; player++) {
//         digitalWrite(players[(player - direction) % numPlayers].light, LOW);
//         digitalWrite(players[player].light, HIGH);
//         delay(speed);
//       }

//       if (bounce) {
//         direction*=-1;
//       }
//     }
// }

// TWO WAY RUNNING ANIMATION //
void twoWayRunner(LightSpeed speed, int numRuns) {
  mgr.addListener(new EvtTimeListener(speed, true, (EvtAction)twoWayLoop));
}

bool twoWayLoop() {
  return false;
}

// void blockingTwoWayRunner(LightSpeed speed, int numRuns) {
//   lightsOff();

//   for(int runs = 0; runs < numRuns; runs++) {
//     for(int player = 0; player < numPlayers - 1; player++) {
//       digitalWrite(players[player].light, HIGH);
//       digitalWrite(players[(numPlayers - 1) - player].light, HIGH);
//       delay(speed);
//       digitalWrite(players[player].light, LOW);
//       digitalWrite(players[(numPlayers - 1) - player].light, LOW);
//     }
//   }
// }

// FLASHING ANIMATION //
bool blinkState = LOW;
int numFlashes = 0;

void flasher(LightSpeed speed, int flashes) {
  lightsOn();
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

// void blockingflasher(LightSpeed speed, int flashes) {
//   lightsOff();
//   for(int numFlashes = 0; numFlashes < flashes; numFlashes++) {
//     lightsOn();
//     delay(speed);
//     lightsOff();
//     delay(speed);
//   }
// }

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

  if (animCtr > sizeof(currentAnim) / sizeof(anim)) {
    animCtr == 0;
  }
  digitalWrite(14, (currentAnim + animCtr)->state);

  animCtr++;
  
  mgr.addListener(new EvtTimeListener((currentAnim + animCtr)->time , false, (EvtAction)stateMonitor));
}

//INTRO SEQUENCE
void doIntro() {
  soundPlayer.play(11); // INTRO SOUND
  // flasher(FAST, 5); // 1.25s  (SPEED * LOOPS)
  // offRunner(MEDIUM, 3); // 1.25s in, 1.25s out, 4.5s loop (7s) (SPEED * PLAYERS * 2 + SPEED * PLAYERS* LOOPS)
  // onRunner(MEDIUM, 2, true, FORWARD); // 6s (SPEED * PLAYERS * LOOPS * 2)
  // onRunner(MEDIUM, 2, true, BACKWARD); // 6s (SPEED * PLAYERS * LOOPS * 2)
  // twoWayRunner(MEDIUM, 4); // 6s (SPEED * PLAYERS * LOOPS)
  // onRunner(MEDIUM, 4, false, FORWARD); // 6s (SPEED * PLAYERS * LOOPS)
  // onRunner(MEDIUM, 4, false, BACKWARD); // 6s (SPEED * PLAYERS * LOOPS)
  // offRunner(MEDIUM, 3); // 1.25s in, 1.25s out, 4.5s loop (7s) (SPEED * PLAYERS * 2 + SPEED * PLAYERS* LOOPS)
  // flasher(FAST, 5); // 1.25s (SPEED * LOOPS)
  soundPlayer.stop();
}
