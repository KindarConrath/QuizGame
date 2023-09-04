//Blink Program
#include <IoAbstraction.h>

bool status = HIGH;
int button = 8;
int led = 13;

void blink() {
  internalDigitalDevice().digitalWriteS(13, status);
  status = !status;
}

void startBlink(pinid_t pin, bool held) {
    if (pin == led) {
        if (held) {
            taskManager.scheduleOnce(0, blink);
        }
        taskManager.scheduleFixedRate(500, blink);
    } else {
        taskManager.scheduleOnce(0, blink);
    }
}

void setup() {
    switches.init(asIoRef(internalDigitalDevice()), SWITCHES_POLL_EVERYTHING, true);

    internalDigitalDevice().pinMode(13, OUTPUT);

    switches.addSwitch(button, startBlink, NO_REPEAT);
}

void loop() {
    taskManager.runLoop();
}
