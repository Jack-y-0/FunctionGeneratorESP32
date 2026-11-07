# Web-Controlled Function Generator

This PlatformIO project adds a web interface for two independently controlled DAC outputs. It supports the classic ESP32 DevKit (`esp32dev`, the default target) and ESP32-S2 (`esp32-s2-saola-1`). The original standalone sketches remain in their existing folders.

## Waveforms and controls

- Sine, square, triangle, sawtooth, synthetic PPG pulse, and DC level
- Independent waveform, frequency, amplitude, offset, and enable control for each channel
- Frequency range: 0.5–10 Hz; PPG mode corresponds to 30–600 beats/minute
- Per-channel waveform preview and DAC/timer diagnostics
- Outputs start disabled after every reboot; enable each channel separately from the webpage
- Settings persist across reboot, but the enabled state never does

## Pins and wiring

| Target | DAC1 | DAC2 |
|---|---:|---:|
| Classic ESP32 (`esp32dev`) | GPIO25 | GPIO26 |
| ESP32-S2 (`esp32-s2-saola-1`) | GPIO17 | GPIO18 |

The built-in DAC outputs are fixed to these pins and cannot be reassigned from the webpage. Outputs are unipolar, nominally 0–3.3 V, and 8-bit; they are not calibrated. Connect generator and receiver grounds together, and connect only to a high-impedance input that accepts this voltage range. Verify with an oscilloscope before connecting other equipment.

## Build and upload

Open `functiongenerator_web` in PlatformIO. The default target is `esp32dev`:

```sh
pio run -e esp32dev
pio run -e esp32dev -t upload
```

For an ESP32-S2 board, select `esp32-s2-saola-1` instead. Do not upload the S2 image to a classic ESP32 or vice versa.

On first setup, the Wi-Fi portal is named `ESP32_MUXSIGSIM`. Its temporary password and the web login (`admin` plus its generated password) are printed to Serial Monitor at 115200 baud. After connecting the device to Wi-Fi, open the IP printed in Serial Monitor from a device on the same network.

## Signal settings

Amplitude is the requested peak-to-peak span as a percentage of nominal DAC range. Offset is the waveform center as a percentage of that range. Firmware clips output to 0–3.3 V; for a 100% amplitude waveform, set offset to 50%. A 0% amplitude waveform is flat. DC mode outputs the selected offset level.

The **Load DC midpoint test** button sets DAC1 to code 128 and DAC2 to code 64; it does not enable either output. Enable only the channel(s) you want to run. The page shows timer progress and the latest DAC code to help distinguish a disabled channel from a stalled signal timer.

## Waveform command IDs

| Command | Waveform |
|---|---|
| `a0` | Sine |
| `a1` | Square |
| `a2` | Triangle |
| `a3` | Sawtooth |
| `a4` | Synthetic PPG pulse |
| `a5` | DC level |

The webpage has a separate waveform button on each channel. USB Serial accepts `ch1` or `ch2` to select a channel, followed by `a0`–`a5` to select its waveform.

## Version history

Git commits identify the versions below; no numbered release scheme has been adopted yet. Dates use Asia/Beirut time. Earlier entries are selected milestones, not the complete Git log.

| Date | Revision | Changes |
|---|---|---|
| 2026-11-08 | This documentation revision | Add version history and a standards proposal for review; no firmware changes. |
| 2026-11-08 | `75c28a1` | Fix controls editing stale channel settings; animate the simulated waveform over a fixed 2-second window so frequency changes affect cycle spacing. |
| 2026-11-07 | `6d05c0d` | Make API authentication explicit and show authentication failures. |
| 2026-11-07 | `ab46196` | Serialize channel updates and display DAC diagnostics. |
| 2026-11-07 | `f766ec5` | Add DAC sample timer and sample diagnostics. |
| 2026-11-07 | `1fadb59` | Remove the BOOT pin gate from the DAC timer. |
| 2026-11-07 | `5199934` | Ignore stale state polling responses during channel edits. |

The animated graph is a simulated preview, not a measurement of the DAC output. See the [standards proposal](docs/standards_proposal.md) for candidate standards awaiting approval.
