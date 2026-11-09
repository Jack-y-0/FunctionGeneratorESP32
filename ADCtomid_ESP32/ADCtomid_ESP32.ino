// DAC level test for classic ESP32 or ESP32-S2.
#include <Arduino.h>
#if defined(CONFIG_IDF_TARGET_ESP32S2)
const int DAC1_PIN = 17;
const int DAC2_PIN = 18;
#elif defined(CONFIG_IDF_TARGET_ESP32)
const int DAC1_PIN = 25;
const int DAC2_PIN = 26;
#else
#error "Supported targets: classic ESP32 and ESP32-S2."
#endif

void setup() {
  dacWrite(DAC1_PIN, 128); // DAC1 midpoint
  dacWrite(DAC2_PIN, 64);  // DAC2 quarter-scale
}

void loop() {
  // put your main code here, to run repeatedly:

}
