#include <IoAbstraction.h>
#include <TaskManagerIO.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

enum Direction {
  FORWARD = 1,
  BACKWARD = -1
};

enum LightSpeed {
  SLOW = 500,
  MEDIUM = 250,
  FAST = 100 
};

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

static const int introSound = 2;
static const int timeoutSound = 1;
static const int thinkingSound = 4;
static const int correctSound = 3;

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // connect to pin 2 on the DFPlayer via a 1K resistor
static const uint8_t PIN_MP3_RX = 3; // connect to pin 3 on the DFPlayer

// Software serial library
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Player
DFRobotDFPlayerMini soundPlayer;

GameState currentState = STARTUP;

void onBuzzerPressed(pinid_t, bool);
void cb1Pressed(pinid_t, bool);
void cb2Pressed(pinid_t, bool);
void stateMonitor();
void doTimeout();
void stopSounds();
void setLights(bool);
void onRunner(LightSpeed, int, bool, Direction);
void onRunNext();
int posMod(int a, int b);
void flasher(LightSpeed, int);
void blink();

taskid_t timeoutTimer = NULL;
taskid_t animation = NULL;

void setup() {
    switches.init(asIoRef(internalDigitalDevice()), SWITCHES_POLL_EVERYTHING, true);

    for(Player player : players) {
        internalDigitalDevice().pinMode(player.button, INPUT_PULLUP);
        internalDigitalDevice().pinMode(player.light, OUTPUT);
        switches.addSwitch(player.button, onBuzzerPressed, NO_REPEAT);
    }

    internalDigitalDevice().pinMode(14, OUTPUT);
    switches.addSwitch(controlButton1, cb1Pressed, NO_REPEAT);
    switches.addSwitch(controlButton2, cb2Pressed, NO_REPEAT);

    stateMonitor();

    // Init serial port for DFPlayer Mini
    softwareSerial.begin(9600);
    // Start communication with DFPlayer Mini
    if (soundPlayer.begin(softwareSerial)) soundPlayer.volume(30);

    soundPlayer.play(introSound);
    onRunner(MEDIUM, 15, true, FORWARD); // 6 * 250 * 15 = 22500ms
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

        soundPlayer.play(currentPlayer.sound);
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
    soundPlayer.stop();
    taskManager.cancelTask(animation);
    animation = NULL;
    setLights(LOW);
    flasher(MEDIUM, 4);
  }
}

void cb2Pressed(pinid_t pin, bool healdDown) {
  if (currentState == ANSWERING || currentState == WAITING) {
    soundPlayer.play(correctSound); //CORRECT ANSWER SOUND
    if (timeoutTimer) {
      taskManager.cancelTask(timeoutTimer);
      timeoutTimer = NULL;
    }
    currentState = PLAYING;
  } else if (currentState == PLAYING) {
    soundPlayer.play(thinkingSound); //THINKING MUSIC
  }
}

struct anim {
  bool state;
  int time;
};

static const anim startupAnim [] = {{HIGH,200}, {LOW,200}}; //constant flashing
static const anim playingAnim [] = {{HIGH,200}, {LOW,500}, {LOW, 500}}; //quick blink, with pause
static const anim answeringAnim [] = {{HIGH,200}, {LOW,200}, {HIGH,500}, {HIGH,500}, {LOW,200}}; //quick blink, long blink
static const anim waitingAnim [] = {{HIGH,500}}; //constant on

GameState prevState = STARTUP;
int stateAnimCtr = 0;

void stateMonitor() {
  const anim *currentAnim;

  if (currentState != prevState) {
    stateAnimCtr = 0;
    prevState = currentState;
  }

  switch(currentState) {
    case STARTUP:
      currentAnim = startupAnim;
      break;
    case PLAYING:
      currentAnim = playingAnim;
      break;
    case ANSWERING:
      currentAnim = answeringAnim;
      break;
    case WAITING:
      currentAnim = waitingAnim;
      break;
  }

  int arrSize = sizeof(currentAnim) / sizeof(anim);
  if (stateAnimCtr > arrSize) {
    stateAnimCtr = 0;
  }
  digitalWrite(14, (currentAnim + stateAnimCtr)->state);

  stateAnimCtr++;

  taskManager.scheduleOnce((currentAnim + stateAnimCtr)->time, stateMonitor);
}

void doTimeout() {
  setLights(LOW);
  if (currentState == ANSWERING) {
    currentState = WAITING;
    soundPlayer.play(timeoutSound); //TIMEOUT SOUND
    timeoutTimer = NULL;
    taskManager.scheduleOnce(3000, stopSounds);
  }
}

void setLights(bool state) {
  for(Player player : players) {
    internalDigitalDevice().digitalWriteS(player.light, state);
  }
}

void stopSounds() {
  soundPlayer.stop();
}

// ONE LIGHT ON RUNNING ANIMATION //
int numAnimLoops = 0;
int animDirection = FORWARD;
bool animBounce = false;
int animPosition = 0;
LightSpeed animSpeed = SLOW;

void onRunner(LightSpeed speed, int numRuns, bool bounce, Direction direction = FORWARD) {
  setLights(LOW);
  animDirection = direction;
  animPosition = animDirection == FORWARD ? 0 : numPlayers - 1;
  animBounce = bounce;
  numAnimLoops = animBounce ? numRuns * 2 : numRuns;
  animSpeed = speed;
  animation = taskManager.scheduleFixedRate(animSpeed, onRunNext);
}

void onRunNext() {
  digitalWrite(players[posMod((animPosition - animDirection), numPlayers)].light, LOW);
  digitalWrite(players[animPosition % numPlayers].light, HIGH);

  animPosition+=animDirection;
  if (animPosition < 0 || animPosition >= numPlayers) {
    if (animBounce) {
      animDirection*=-1;
    }
    animPosition = animDirection == FORWARD ? 0 : numPlayers - 1;
    numAnimLoops--;
  }

  if (numAnimLoops < 0) {
    taskManager.cancelTask(animation);
    flasher(MEDIUM, 4);
  }
}

int posMod(int a, int b) {
  return (b + (a % b)) % b;
}

// FLASHING ANIMATION //
bool blinkState = LOW;

void flasher(LightSpeed speed, int flashes) {
  setLights(HIGH);
  numAnimLoops = flashes * 2;
  blinkState = LOW;
  animation = taskManager.scheduleFixedRate(speed, blink);
}

void blink() {
  setLights(blinkState);
  blinkState = !blinkState;

  numAnimLoops--;
  if (numAnimLoops <= 0) {
    taskManager.cancelTask(animation);
  }
}
