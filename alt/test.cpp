// TEST FILE
#include <Arduino.h>
#include <Eventually.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

void enableButtons();
void onPlayerPress1();
void onPlayerPress2();
void onPlayerPress3();
void onPlayerPress4();
void onPlayerPress5();
void onPlayerPress6();
void onPlayerPress7();
void onPlayerPress8();
void doTimeout();
void startup();
void resetLights();
void setupPins();

EvtManager mgr;

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

static const int lights [6] = {8, 9, 10, 11, 12, 13};

bool first = true;
bool second = false;

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // connect to pin 2 on the DFPlayer via a 1K resistor
static const uint8_t PIN_MP3_RX = 3; // connect to pin 3 on the DFPlayer

// Software serial library
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Player
DFRobotDFPlayerMini soundPlayer;

EvtTimeListener* timer;

void setup() {
    setupPins();

    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(13, HIGH);

    // Init serial port for DFPlayer Mini
    softwareSerial.begin(9600);
    // Start communication with DFPlayer Mini
    if (soundPlayer.begin(softwareSerial)) soundPlayer.volume(30);

    resetLights();

    soundPlayer.play(11);
    startup();

    enableButtons();

    resetLights();
    soundPlayer.stop();
}

void setupPins() {
    for (int ctr = 8; ctr <= 13; ctr++) {
        pinMode(ctr, OUTPUT);
    }

    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);
    pinMode(A2, INPUT_PULLUP);
    pinMode(A3, INPUT_PULLUP);
    pinMode(A4, INPUT_PULLUP);
    pinMode(A5, INPUT_PULLUP);
    pinMode(1, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
}

void enableButtons() {
  mgr.addListener(new EvtPinListener(A0, (EvtAction)onPlayerPress1));
  mgr.addListener(new EvtPinListener(A1, (EvtAction)onPlayerPress2));
  mgr.addListener(new EvtPinListener(A2, (EvtAction)onPlayerPress3));
  mgr.addListener(new EvtPinListener(A3, (EvtAction)onPlayerPress4));
  mgr.addListener(new EvtPinListener(A4, (EvtAction)onPlayerPress5));
  mgr.addListener(new EvtPinListener(A5, (EvtAction)onPlayerPress6));
  mgr.addListener(new EvtPinListener(1, (EvtAction)onPlayerPress7));
  mgr.addListener(new EvtPinListener(0, (EvtAction)onPlayerPress8));
}

void startup() {
    digitalWrite(8, HIGH);
    delay(500);
    digitalWrite(9, HIGH);
    delay(500);
    digitalWrite(10, HIGH);
    delay(500);
    digitalWrite(11, HIGH);
    delay(500);
    digitalWrite(12, HIGH);
    delay(500);
    digitalWrite(13, HIGH);
    delay(500);

    resetLights();
    delay(500);

    digitalWrite(8, HIGH);
    delay(250);
    digitalWrite(9, HIGH);
    delay(250);
    digitalWrite(10, HIGH);
    delay(250);
    digitalWrite(11, HIGH);
    delay(250);
    digitalWrite(12, HIGH);
    delay(250);

    for (int ctr = 0; ctr < 6; ctr++) {
        digitalWrite(lights[ctr], LOW);
        digitalWrite(lights[(ctr - 1) % 6], HIGH);
        delay(250);
    }
    digitalWrite(lights[5], HIGH);
    delay(1000);

    resetLights();
}

USE_EVENTUALLY_LOOP(mgr)

void onPlayerPress1() {
    soundPlayer.play(1);
    digitalWrite(8, HIGH);
    delay(500);
    digitalWrite(8, LOW);
    soundPlayer.stop();
}

void onPlayerPress2() {
    soundPlayer.play(2);
    digitalWrite(9, HIGH);
    delay(500);
    digitalWrite(9, LOW);
    soundPlayer.stop();
}

void onPlayerPress3() {
    soundPlayer.play(3);
    digitalWrite(10, HIGH);
    delay(500);
    digitalWrite(10, LOW);
    soundPlayer.stop();
}

void onPlayerPress4() {
    soundPlayer.play(4);
    digitalWrite(11, HIGH);
    delay(500);
    digitalWrite(11, LOW);
    soundPlayer.stop();
}

void onPlayerPress5() {
    soundPlayer.play(5);
    digitalWrite(12, HIGH);
    delay(500);
    digitalWrite(12, LOW);
    soundPlayer.stop();
}

void onPlayerPress6() {
    soundPlayer.play(6);
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    soundPlayer.stop();
}

void onPlayerPress7() {
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

void onPlayerPress8() {
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
            timer = new EvtTimeListener(1000, true, (EvtAction)doTimeout);
            mgr.addListener(timer);
            second = false;
        } else {
            mgr.removeListener(timer);
            second = true;
        }
    }
}

void doTimeout() {
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH);
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    digitalWrite(13, LOW);
    second = true;
}

void resetLights() {
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    digitalWrite(13, LOW);
}
