// TEST FILE
#include <IoAbstraction.h>
#include <TaskManagerIO.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

void enableButtons();
void onPlayerPressed(pinid_t pin, bool healdDown);
void onCB1Pressed(pinid_t pin, bool healdDown);
void onCB2Pressed(pinid_t pin, bool healdDown);
void doTimeout();
void startup();
void setLights(bool state);
void setupPins();
int posMod(int, int);

enum GameState {
  Startup,
  Playing,
  Answering,
  Waiting
};

struct Player {
  unsigned char button;
  unsigned char light;
  unsigned char sound;
};

static const int numPlayers = 6;
static const Player players [numPlayers] = {{A0, 8, 5}, {A1, 9, 6}, {A2, 10, 7}, {A3, 11, 8}, {A4, 12, 9}, {A5, 13, 10}};
static const int controlButtons [2] = {0, 1};
static const int boxLight = 14;

bool first = true;
bool second = false;

taskid_t timer;

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // connect to pin 2 on the DFPlayer via a 1K resistor
static const uint8_t PIN_MP3_RX = 3; // connect to pin 3 on the DFPlayer

// Software serial library
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Player
DFRobotDFPlayerMini soundPlayer;

void setup() {
    setupPins();

    setLights(HIGH);

    // Init serial port for DFPlayer Mini
    softwareSerial.begin(9600);
    // Start communication with DFPlayer Mini
    if (soundPlayer.begin(softwareSerial)) soundPlayer.volume(30);

    setLights(LOW);

    delay(500);
    soundPlayer.play(1);
    startup();

    enableButtons();

    setLights(LOW);
    soundPlayer.stop();
}

void loop() {
    taskManager.runLoop();
}

void setupPins() {
    for (Player player : players) {
        internalDigitalDevice().pinMode(player.light, OUTPUT);
        internalDigitalDevice().pinMode(player.button, INPUT_PULLUP);
    }

    internalDigitalDevice().pinMode(boxLight, OUTPUT);
    internalDigitalDevice().pinMode(controlButtons[0], INPUT_PULLUP);
    internalDigitalDevice().pinMode(controlButtons[1], INPUT_PULLUP);
}

void enableButtons() {
    switches.init(asIoRef(internalDigitalDevice()), SWITCHES_POLL_EVERYTHING, true);

    for (Player player : players) {
        switches.addSwitch(player.button, onPlayerPressed, NO_REPEAT);
    }

    switches.addSwitch(controlButtons[0], onCB1Pressed, NO_REPEAT);
    switches.addSwitch(controlButtons[1], onCB2Pressed, NO_REPEAT);
}

void setLights(bool state) {
    for (Player player : players) {
        internalDigitalDevice().digitalWriteS(player.light, state);
    }
    internalDigitalDevice().digitalWriteS(boxLight, state);
}

void startup() {
    for (Player player : players) {
        internalDigitalDevice().digitalWriteS(player.light, HIGH);
        delay(500);
    }

    setLights(LOW);
    delay(500);

    for (Player player : players) {
        internalDigitalDevice().digitalWriteS(player.light, HIGH);
        delay(500);
    }

    for (int ctr = 0; ctr < 6; ctr++) {
        digitalWrite(players[ctr].light, LOW);
        digitalWrite(players[posMod(ctr - 1, 6)].light, HIGH);
        delay(250);
    }
    digitalWrite(players[5].light, HIGH);
    delay(1000);

    setLights(LOW);
}

void onPlayerPressed(pinid_t pin, bool healdDown) {
    Player player;
    for (Player p : players) {
        if (p.button == pin) {
            player = p;
            break;
        }
    }

    soundPlayer.play(player.sound);
    digitalWrite(player.light, HIGH);
    delay(500);
    digitalWrite(player.light, LOW);
    soundPlayer.stop();
}

void onCB1Pressed(pinid_t pin, bool healdDown) {
    soundPlayer.play(9);
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    delay(500);
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    soundPlayer.stop();
}

void onCB2Pressed(pinid_t pin, bool healdDown) {
    if (first) {
        soundPlayer.play(14);
        digitalWrite(11, HIGH);
        digitalWrite(12, HIGH);
        digitalWrite(13, HIGH);
        delay(500);
        digitalWrite(11, LOW);
        digitalWrite(12, LOW);
        digitalWrite(13, LOW);
        soundPlayer.stop();
        first = false;
        second = true;
    } else {
        if (second) {
            timer = taskManager.scheduleOnce(1000, doTimeout);
            second = false;
        } else {
            if (taskManager.getTask(timer)) {
                taskManager.cancelTask(timer);
                timer = NULL;
            }
            second = true;
        }
    }
}

void doTimeout() {
    timer = NULL;
    setLights(HIGH);
    delay(500);
    setLights(LOW);
    second = true;
}

int posMod(int a, int b) {
  return (b + (a % b)) % b;
}
