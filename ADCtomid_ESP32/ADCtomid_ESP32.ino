// set esp32 ADC to midpoint (2047) on GPIO25, different value on GPIO26 (1027)

void setup() {
  dacWrite(25, 128); // Sets GPIO25 to midrange
  dacWrite(26, 64); // Sets GPIO26 to midrange   
}

void loop() {
  // put your main code here, to run repeatedly:

}
