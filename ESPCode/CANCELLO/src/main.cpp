
#include <Arduino.h>
#include <Cancello.h>

Cancello cancello;




void setup() {
  cancello.begin();
}

void loop() {
  cancello.loop();
}


