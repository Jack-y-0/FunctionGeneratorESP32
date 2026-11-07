# Web-Controlled Function Generator (ESP32-S2)

This is an additional sketch for the existing `FunctionGeneratorESP32` repository. It leaves the original v0.0.6 sketch in `functiongenerator_esp32/` unchanged and adds browser control for two independent outputs.

## Waveforms and controls

- Sine, square, triangle, sawtooth, synthetic PPG pulse, and DC level
- Independent waveform, frequency, amplitude, offset, and enable controls for each DAC channel
- Frequency range: 0.5–10 Hz; in PPG mode this corresponds to 30–600 beats/minute
- Live waveform preview in the webpage
- DC midpoint test preset: GPIO17 at 128/255 and GPIO18 at 64/255
- BOOT button pauses output updates, preserving the existing sketch's pause behavior

Defaults are PPG at 1.2 Hz (72 beats/minute) on both outputs; Channel 2 starts at half the peak-to-peak amplitude of Channel 1, matching the existing sketch's intent.

## ESP32-S2 wiring

| Generator output | Pin |
|---|---|
| DAC channel 1 | GPIO17 |
| DAC channel 2 | GPIO18 |
| Common reference | GND |

Connect the outputs only to high-impedance analog inputs that accept 0–3.3 V. Connect the generator ground to the receiving board ground. The ESP32-S2 DAC is 8-bit, unipolar, and not a calibrated function-generator output. Check the signal with a scope before using it with other hardware.

## Build and upload with PlatformIO

1. Open the `functiongenerator_web` folder in VS Code with PlatformIO installed.
2. Connect the ESP32-S2 WROOM DevKit 1 over USB.
3. Build and upload the `esp32-s2-saola-1` environment. PlatformIO installs WiFiManager from `platformio.ini`.
4. Open the Serial Monitor at 115200 baud.
5. On first boot, connect your phone or computer to `ESP32-S2-WaveGen-Setup`. If its setup page does not open automatically, go to `192.168.4.1`.
6. Select your Wi-Fi network and enter its password.
7. After the ESP32 connects, use the local IP printed in Serial Monitor. Open `http://<ESP32-IP>` on a device connected to the same Wi-Fi network.

The network credentials and waveform settings are not stored in the sketch. WiFiManager saves the Wi-Fi network; waveform settings return to the defaults after a restart.

## Amplitude and offset

Amplitude is the requested peak-to-peak span, as a percentage of the nominal 0–3.3 V DAC range. Offset sets its center voltage as a percentage of the same range. Firmware clips values to 0–3.3 V. For an unclipped 100% amplitude waveform, set offset to 50%.

## Uploading the combined firmware

Upload only this PlatformIO project to use the web controls and all waveform modes. The two older `.ino` folders are standalone sketches and must not be compiled as part of this same firmware. Their useful behavior has been incorporated here: the synthetic PPG shape is a selectable waveform, and the fixed DAC test levels are available through the **Load DC midpoint test** preset. That preset sets GPIO17 to code 128 (about half scale) and GPIO18 to code 64 (about quarter scale). The `ADCtomid_ESP32` sketch writes DAC levels; it does not read ADC inputs.
