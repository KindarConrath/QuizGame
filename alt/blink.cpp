//Blink Program
#include <IoAbstraction.h>

bool status = HIGH;

void blink() {
  internalDigitalDevice().digitalWriteS(13, status);
  status = !status;
}

void setup() {
    switches.init(asIoRef(internalDigitalDevice()), SWITCHES_POLL_EVERYTHING, true);

    internalDigitalDevice().pinMode(13, OUTPUT);

    taskManager.scheduleFixedRate(500, blink);
}

void loop() {
    taskManager.runLoop();
}
