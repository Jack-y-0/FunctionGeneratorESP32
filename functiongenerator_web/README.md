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

Wi-Fi setup uses a unique SoftAP name in the format `ESP32_<station MAC without colons>`; for example, MAC `AA:BB:CC:DD:EE:FF` gives `ESP32_AABBCCDDEEFF`. The name is stable for each device and uses the same station MAC shown on the splash screen. The temporary setup password and generated web password (`admin`) are printed to Serial Monitor at 115200 baud. The **Wave generator** view uses one card per DAC output, with an individual waveform preview and independent waveform, frequency, amplitude, offset, and output-enable controls. The **Oscilloscope inputs** view uses a full-width graph with selectable ADC or PWM channels; selected ADC traces share one graph, and selected PWM traces use separate lanes within one graph. This input-scope layout does not change the DAC generator controls. The ADC time window is limited to the 1.28-second sample buffer; the PWM trace is reconstructed from measured edge timing. The generator preview animates over a fixed 2-second window while output is enabled. Disabled outputs show a flat trace; the graph is labeled as a simulated preview and does not measure the DAC output.

## Debug verification

Run the browser-logic regression checks without connecting a board:

```sh
node --test tests/web_regression.cjs
```

These checks cover generator controls after state replacement, preview frequency and animation, stale PWM polling, and ordered PWM configuration requests. They use a mocked page/API and do not verify rendered browser layout or physical signal accuracy.

## Device-information splash screen

Opening the web interface displays a dismissible splash with the program name,
full patch version, firmware compile date/time, Wi-Fi station MAC address, and
connected Wi-Fi SSID. Choose **Continue to generator** or press Escape to dismiss
it; **Device info** in the header opens it again. Viewing or dismissing the splash
does not enable outputs or change generator settings.

The information comes from the authenticated `/api/state` response. Compile time
is embedded using `__DATE__` and `__TIME__` from the build machine, rather than the
browser clock or device boot time. Wi-Fi status refreshes with the existing
five-second state poll. Disconnected devices report **Not connected**; failed
requests show **Unavailable** and retry automatically. Older firmware without
the new metadata reports missing fields explicitly. The splash uses the same
firmware identifier as the connection badge, compatible with automatic version
bumps when the versioning workflow is merged.

## Version history

| Date (Asia/Beirut) | Revision | Changes |
|---|---|---|
| 2026-11-08 | This debug revision | Restore current-state generator controls and animated frequency-dependent preview; serialize PWM configuration and reject stale selection updates; correct ADC zero label; add four regression checks. |
| 2026-11-08 | `e03a067` | Restore per-channel generator layout. |
| 2026-11-08 | `450c363` | Separate hardware and software work in the [Safety Gap assesment](docs/Safety%20Gap%20assesment.md). |
| 2026-11-08 | `194a978` | Document version history and proposed instrument standards. |
| 2026-11-08 | `75c28a1` | Earlier generator control and animated-preview fixes, subsequently regressed during layout changes. |

Entries are selected Git milestones, not numbered releases. Proposed standards remain awaiting approval.

