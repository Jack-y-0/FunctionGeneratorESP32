# Web-Controlled Function Generator and Scope — Classic ESP32

This feature branch targets the classic ESP32 DevKit (`esp32dev`) selected for the project. It combines the existing two-channel DAC waveform generator with six analog ADC scope inputs and 17 configurable digital PWM edge-capture inputs. Channel 1 and Channel 2 output controls remain independent; both outputs start disabled after reboot.

## Scope capacity

- **Analog:** GPIO32, 33, 34, 35, 36, and 39 (six exposed ADC1 inputs on the common ESP32-WROOM DevKit). Samples are read in sequence at about 200 frames/s per input and displayed as raw 12-bit counts.
- **Digital PWM:** GPIO4, 13, 14, 16, 17, 18, 19, 21, 22, 23, 27, 32, 33, 34, 35, 36, and 39. The page lets you select one or more pins for capture and overlays their reconstructed pulse traces on a shared full-width display; frequency and duty cycle are reported in the legend. Analog and digital inputs are selected in separate modes.

The analog ADC channels are multiplexed, not simultaneous. ADC2 is not included because the webpage uses Wi-Fi. PWM edge capture is for low-frequency signal checks, not a high-speed logic analyzer.

## Electrical limits

Never apply a negative voltage or more than 3.3 V to an ESP32 input; GPIOs are not 5 V tolerant. At the configured ADC attenuation, full scale may be around 3.1 V and varies by chip. ADC results are uncalibrated. Use an appropriately protected attenuator/buffer for external signals and verify actual values with a scope or meter.

## Upload

```sh
pio run -e esp32dev
pio run -e esp32dev -t upload
```

Wi-Fi setup uses `ESP32_MUXSIGSIM`. The temporary setup password and generated web password (`admin`) are printed to Serial Monitor at 115200 baud. The **Wave generator** view uses one card per DAC output, with an individual waveform preview and independent waveform, frequency, amplitude, offset, and output-enable controls. The **Oscilloscope inputs** view uses a full-width graph with selectable ADC or PWM channels; selected ADC traces share one graph, and selected PWM traces use separate lanes within one graph. This input-scope layout does not change the DAC generator controls. The ADC time window is limited to the 1.28-second sample buffer; the PWM trace is reconstructed from measured edge timing. The generator preview remains visible while output is off and is labeled “preview only.”
