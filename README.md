
<img width="803" height="890" alt="Screenshot 2026-11-08 at 00 28 12" src="https://github.com/user-attachments/assets/d389e689-68d3-4dfb-b60f-30eee0b7d29a" />

# FunctionGeneratorESP32


- `functiongenerator_web/` — the two-channel web-controlled generator. Use this PlatformIO project for the webpage and independent channel controls.

## Web-controlled generator

The web firmware supports sine, square, triangle, sawtooth, synthetic PPG, and DC outputs. Channel 1 and Channel 2 have independent waveform, frequency, amplitude, offset, and enable settings. Both outputs start disabled after boot; enable each output on its webpage card.

The default PlatformIO target is `esp32dev` for the classic ESP32 DevKit. Its DAC pins are GPIO25 and GPIO26. The S2 target uses GPIO17 and GPIO18. The webpage displays the selected target's pin mapping. See [`functiongenerator_web/README.md`](functiongenerator_web/README.md) for setup, credentials, wiring, and diagnostics.

## Uploading

Upload only `functiongenerator_web/` to run the web interface. The standalone `.ino` projects each have their own `setup()` and `loop()` and are not compiled together with the web firmware. The BOOT-button pause behavior belongs to the original standalone sketch; web outputs are controlled independently from the webpage.
