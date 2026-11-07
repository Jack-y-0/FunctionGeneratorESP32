# Web-Controlled Function Generator

This is the combined PlatformIO firmware for the FunctionGeneratorESP32 project. It supports both the classic ESP32 DevKit and the ESP32-S2 WROOM DevKit 1.

## Waveforms and controls

- Sine, square, triangle, sawtooth, synthetic PPG pulse, and DC level
- Separate waveform buttons and frequency, amplitude, offset, and enable controls for both DAC outputs
- Frequency range: 0.5–10 Hz; in PPG mode this corresponds to 30–600 beats/minute
- USB Serial commands use the same waveform IDs as the webpage
- DC midpoint preset reproduces DAC1 code 128 and DAC2 code 64
- BOOT button pauses waveform updates

## Board targets and DAC pins

| PlatformIO environment | Board | DAC1 | DAC2 |
|---|---|---|---|
| `esp32dev` | Classic ESP32 | GPIO25 | GPIO26 |
| `esp32-s2-saola-1` | ESP32-S2 | GPIO17 | GPIO18 |

The upload log you shared identifies a classic ESP32, so select `esp32dev`. The webpage obtains the pin mapping from firmware and displays the correct GPIO on each channel card.

Connect the DAC pins to high-impedance analog inputs that accept 0–3.3 V, and connect the boards' grounds together. The DAC is 8-bit, unipolar, and not a calibrated function-generator output. Check the signal with a scope before connecting other equipment.

## Build and upload with PlatformIO

1. Open the `functiongenerator_web` folder in VS Code with PlatformIO installed.
2. Connect the board over USB.
3. Select `esp32dev` for a classic ESP32 or `esp32-s2-saola-1` for an ESP32-S2.
4. Build and upload that environment. PlatformIO installs WiFiManager from `platformio.ini`.
5. Open Serial Monitor at 115200 baud.
6. On first boot, connect your phone or computer to `ESP32-WaveGen-Setup`. If the setup page does not open automatically, browse to `192.168.4.1`.
7. Select your normal Wi-Fi network and enter its password.
8. Open the local IP printed in Serial Monitor from a device on the same Wi-Fi network.

## Waveform command IDs

| Command | Selection |
|---|---|
| `a0` | Sine |
| `a1` | Square |
| `a2` | Triangle |
| `a3` | Sawtooth |
| `a4` | Synthetic PPG pulse |
| `a5` | DC level |

Each channel card has one button per command. In USB Serial Monitor, send `a0`–`a5` to change the selected serial channel. Use `ch1` for DAC1 or `ch2` for DAC2 first. Set the line ending to Newline or Both NL & CR.

## Pin selection

The two built-in DACs have fixed pins for each board family: GPIO25/26 on classic ESP32, GPIO17/18 on ESP32-S2. The webpage lets you select a waveform independently for each DAC output and displays the selected pins. It cannot reroute the built-in DACs to arbitrary GPIOs. Other GPIOs can output PWM, but using PWM as an analog-like output requires a low-pass filter and is a separate mode.

## Project structure

The original PPG and DAC midpoint sketches remain as standalone examples. Upload only this PlatformIO project to run the combined webpage, PPG, waveform, and DC test features.