#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Hello");
}

void loop() {
  Serial.println("World!");
  delay(1000);
}
