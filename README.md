# FunctionGeneratorESP32

This repository contains the original standalone waveform sketch and the web-controlled firmware in `functiongenerator_web/`.

The `feature/multichannel-scope` branch targets the classic ESP32 DevKit (`esp32dev`). It combines two independently controlled DAC outputs with six ADC1 analog scope channels and 17 configurable digital PWM edge-capture inputs. The scope view selects ADC or PWM inputs and overlays selected channels on a full-width graph. The generator remains organized as independent DAC channel cards with one preview per channel. Both DAC outputs start disabled after reboot.

See [`functiongenerator_web/README.md`](functiongenerator_web/README.md) for the pin lists, sampling behavior, electrical limits, Wi-Fi setup, and upload instructions. The original `.ino` sketches remain standalone examples.
