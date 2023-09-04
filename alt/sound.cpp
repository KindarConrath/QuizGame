//Blink Program
#include <IoAbstraction.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

bool status = HIGH;
int button = 8;
int led = 13;

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // connect to pin 2 on the DFPlayer via a 1K resistor
static const uint8_t PIN_MP3_RX = 3; // connect to pin 3 on the DFPlayer

// Software serial library
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

// Player
DFRobotDFPlayerMini soundPlayer;

void playSound(pinid_t pin, bool held) {
    if (pin == button) {
        soundPlayer.play(1);
    }
}

void blink() {
  internalDigitalDevice().digitalWriteS(13, status);
  status = !status;
}

void setup() {
    switches.init(asIoRef(internalDigitalDevice()), SWITCHES_POLL_EVERYTHING, true);

    internalDigitalDevice().pinMode(13, OUTPUT);

    taskManager.scheduleFixedRate(500, blink);

    // Init serial port for DFPlayer Mini
    softwareSerial.begin(9600);
    // Start communication with DFPlayer Mini
    if (soundPlayer.begin(softwareSerial)) soundPlayer.volume(30);

    switches.addSwitch(button, playSound, NO_REPEAT);
}

void loop() {
    taskManager.runLoop();
}
