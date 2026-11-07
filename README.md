# FunctionGeneratorESP32

Uses the DAC outputs on an ESP32 to make a waveform similar to a PPG sensor.  
Analog outputs on the ESP32-S2: GPIO17 (DAC1) and GPIO18 (DAC2).  
The onboard LED is on GPIO2 and is PWM modulated for a similar effect for the user.  

<img width="824" height="646" alt="image" src="https://github.com/user-attachments/assets/add5d9ad-cf85-42c7-88f9-86aaa4b4aa68" />  

User can press and hold the BOOT button to pause the waveform update.  
<img width="824" height="646" alt="image" src="https://github.com/user-attachments/assets/f2217349-7e44-4ea8-b7af-c8f74206d137" />




## Web-controlled generator

The `functiongenerator_web/` folder adds a Wi-Fi webpage for two independently controlled DAC outputs. It offers sine, square, triangle, sawtooth, and the existing synthetic PPG pulse shape. See [`functiongenerator_web/README.md`](functiongenerator_web/README.md) for build and wiring instructions.

## Which sketch should I upload?

Each sketch folder is a separate firmware with its own `setup()` and `loop()`. To use the combined webpage, upload only the PlatformIO project in `functiongenerator_web/`. It includes the four standard waveforms, the PPG pulse, and a DC test preset that reproduces the fixed output levels from `ADCtomid_ESP32/`. The original sketches remain separate examples; they are not compiled together.

The web page provides one waveform-command button group per DAC output. Command IDs are `a0`–`a5`; USB Serial can use the same IDs after selecting `ch1` or `ch2`. The ESP32-S2 DAC pins are fixed at GPIO17 and GPIO18.
