# FunctionGeneratorESP32

This repository contains the original standalone waveform sketch and the web-controlled firmware in `functiongenerator_web/`.

The `feature/multichannel-scope` branch targets the classic ESP32 DevKit (`esp32dev`). It combines two independently controlled DAC outputs with six ADC1 analog scope channels and 17 configurable digital PWM edge-capture inputs. The scope view selects ADC or PWM inputs and overlays selected channels on a full-width graph. The generator remains organized as independent DAC channel cards with one preview per channel. Both DAC outputs start disabled after reboot.

See [`functiongenerator_web/README.md`](functiongenerator_web/README.md) for the pin lists, sampling behavior, electrical limits, Wi-Fi setup, and upload instructions. The original `.ino` sketches remain standalone examples.

## Fun with Functions.
Using some resistors, the function generator, and the oscilloscope we can have some fun with functions.

Lets add two functions through equal resistors (10K) and set the frequencies almost the same.  
<img width="400"  alt="image" src="https://github.com/user-attachments/assets/a3a3a199-ae50-46a2-a886-85b03ac2377e" />

At the ESP32 oscilloscope we get a wave form that slowly changes size.  
<img width="400"  alt="image" src="https://github.com/user-attachments/assets/19f0c000-a51c-4345-8a5f-c8e5cac7c008" />

Using an external oscilloscope (and adding useing the MATH function) displayingwith a longer time base we get:  
<img width="400" alt="image" src="https://github.com/user-attachments/assets/edec834c-d14d-450c-b38b-d7892bbb8cda" />



