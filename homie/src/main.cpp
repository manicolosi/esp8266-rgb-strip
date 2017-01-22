#include <Homie.h>

void setup() {
  Serial.begin(115200);
  Homie.setFirmware("bare-minimum", "1.0.0"); // The "_" is not a typo! See Magic bytes
  Homie.setup();
}

void loop() {
  Homie.loop();
}
