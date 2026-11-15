# Bench verification and output interface

## Purpose and limits

Use this procedure to characterize the ESP32 outputs on a bench before connecting them to test equipment. The built-in DAC is 8-bit and uncalibrated. Record measured values; do not treat the webpage's nominal 0–3.3 V range as a calibration statement. This simulator is for bench signal testing, not connection to a person.

## Scope setup

1. Use a high-impedance 10× probe and a short ground connection.
2. Connect scope CH1 to DAC1 and scope CH2 to DAC2. Connect scope ground to the generator GND. Do not attach the probe ground to a signal output.
3. Keep the output unloaded or connected only to a known high-impedance test input.
4. Start with both channels disabled. Enable one channel at a time for the single-channel measurements, then enable both for the two-channel check.
5. Record the ESP32 board type, firmware commit, scope/probe settings, room/bench setup, and date.

## Measurements per output

| Check | Suggested settings | Record |
|---|---|---|
| Zero/off output | Disable the channel | DC voltage and any residual waveform |
| DC level | Select DC; try offset 10%, 50%, and 90% | Mean voltage, min/max, offset error |
| Frequency | Select sine at 0.5, 1.2, and 10 Hz | Set frequency, measured frequency, percent error |
| Audio frequency | Select sine at 20, 440, 1,000, 10,000, and 20,000 Hz | Actual frequency, amplitude, stair steps, distortion, and any dropouts |
| Amplitude | Select sine at 50% and 100% Vpp, offset 50% | Set amplitude, measured Vpp |
| Shape | Inspect sine, square, triangle, sawtooth, and PPG at 1.2 Hz | Clipping, stair steps, overshoot/ringing |
| Noise | Use DC output at 50%; use a consistent vertical scale and scope bandwidth limit | RMS and peak-to-peak noise |
| Two-channel interaction | Run different waveforms/frequencies on CH1 and CH2 together | Frequency change, amplitude change, crosstalk |
| Wi-Fi timing | Repeat frequency checks while page polling and controls are active | Frequency stability/jitter, missed or uneven updates |

The ESP32's native DAC outputs are not expected to produce precision instrument waveforms. Agree on acceptable frequency error, noise, and voltage error before treating these checks as pass/fail.

## Audio-range verification (issue #4)

The web firmware offers 0.5 Hz through 20 kHz using I2S0/DMA at a nominal 200 kframes/s for both DAC channels. At the upper endpoint there are ten samples per cycle. Frequency accuracy depends on the physical clock; usable amplitude, distortion, and waveform fidelity depend on the DAC, load, and output circuit. The internal ADC scope cannot verify audio signals at its 200 frames/s sampling rate.

1. Start with a sine wave, offset 50%, a low amplitude, and a high-impedance external scope input. Sweep the frequencies in the table and record actual frequency and Vpp on each DAC pin.
2. Repeat with both channels enabled at different frequencies. Confirm channel 1 is GPIO25 and channel 2 is GPIO26; use distinct DC levels as an additional channel-mapping check.
3. Inspect square, triangle, sawtooth, and PPG signals separately. Record reduced detail, aliasing, sampling images, and clipping rather than assuming the 20 kHz sine result applies to every waveform.
4. Keep the webpage polling, change settings repeatedly, and enable scope capture while observing the external scope for gaps or bursts. Include the delayed settings-save period. Record `/api/state` diagnostics, but do not treat a zero write-error counter as proof that no underflows occurred.
5. Disable each output and measure the settling time to zero. Check that restarting leaves both outputs disabled and that the GPIO2 heartbeat remains at 72 BPM.
6. Before listening, confirm the receiving line input's permitted level, impedance, and DC tolerance. Design attenuation, DC blocking, and protection for that input, and verify the conditioned signal on the scope. Use a powered amplifier with low volume initially; never connect a speaker or headphones directly to a DAC pin. A 1 kOhm series resistor alone is not a complete line-output interface.

Record the board, firmware revision, waveform, both channel settings, load, output circuit, Wi-Fi activity, and scope/probe setup with each result. Hardware audio performance has not been established by compilation or software regression tests.

## Output interface design status

The firmware exposes fixed DAC pins for each supported chip: GPIO25/26 on classic ESP32 and GPIO17/18 on ESP32-S2. Arbitrary GPIO reassignment is not supported for DAC mode. PWM outputs, if added later, must be identified separately from DAC outputs.

A protected bench-output path will likely need a buffer, output-current limiting, and protection selected for the receiving equipment. Component values and connector choice are intentionally left open until the target input requirements are known. A generic circuit cannot guarantee compatibility with an unknown medical-equipment input.
