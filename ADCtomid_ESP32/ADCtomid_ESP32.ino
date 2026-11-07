// ESP32-S2 DAC level test: midpoint on GPIO17 and quarter-scale on GPIO18

void setup() {
  dacWrite(17, 128); // ESP32-S2 DAC1: midpoint
  dacWrite(18, 64);  // ESP32-S2 DAC2: quarter-scale
}

void loop() {
  // put your main code here, to run repeatedly:

}
