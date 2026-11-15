# ESP32 Function Generator and Scope

Browser-controlled waveform generator and input scope for the **classic ESP32 DevKit** (`esp32dev`). Includes two independent DAC outputs, six analog inputs, and 17 selectable PWM capture inputs. Both outputs start disabled after every reboot.

The onboard LED on **GPIO2** pulses continuously at **72 BPM**, including during Wi-Fi setup and when both DAC outputs are disabled. Its brightness follows a PPG-style pulse independently of channel settings and timed faults. This indicator requires a board with an active-high LED connected to GPIO2.

## Requirements

- Classic ESP32 DevKit and a USB data cable.
- [Visual Studio Code](https://code.visualstudio.com/) with the official [PlatformIO IDE extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide).
- A 2.4 GHz Wi-Fi network and a phone or computer with a browser.
- Node.js only for running the browser-logic regression tests.

PlatformIO IDE includes its own command-line tools; a separate PlatformIO installation is unnecessary. See the [official VS Code setup guide](https://docs.platformio.org/en/latest/integration/ide/vscode.html).

## Set Up in VS Code

1. Install VS Code, then open **Extensions** and install **PlatformIO IDE** (`platformio.platformio-ide`). Allow its initial setup to finish and reload VS Code if prompted.
2. Select **File > Open Folder** and open `functiongenerator_web`, the folder containing `platformio.ini`. Open the existing project; there is no need to create a new one.
3. Connect the ESP32 using a USB data cable. PlatformIO downloads the configured platform, framework, and WiFiManager library when needed.
4. Open the **PlatformIO** sidebar. Under **PROJECT TASKS > esp32dev > General**, select **Build** and wait for `SUCCESS`.
5. Select **Upload** in the same task group and wait for `SUCCESS`.
6. Select **Monitor** to open Serial Monitor. This project sets its speed to **115200 baud**. Press the board's **EN/RESET** button if you missed the startup messages.

The PlatformIO status-bar buttons also provide Build, Upload, and Serial Monitor actions. Hover over a button to see its name.

## Connect Wi-Fi and Open the Webpage

1. On first startup, or if the saved network cannot be reached, connect your phone or computer to the device network named **ESP32_<MAC without colons>** (for example, **ESP32_AABBCCDDEEFF**). Serial Monitor prints the exact name. This setup network has **no password**. Stay connected if your device reports that it has no internet access.
2. Open the WiFiManager portal when prompted. If it does not open automatically, visit [http://192.168.4.1](http://192.168.4.1).
3. Choose your 2.4 GHz router network, enter **the router's Wi-Fi password** if required, and save. The setup portal times out after three minutes; the ESP32 then restarts so you can try again.
4. Reconnect your phone or computer to the same router network as the ESP32.
5. In VS Code's Serial Monitor, find `Connected to Wi-Fi. Open http://` followed by the device IP address. Open that address in your browser.

The webpage and API require **no username or password**. Anyone on the connected network can access the controls. Saved Wi-Fi credentials are reused on later boots, so the setup network normally disappears after connection.

## Use the Controls

### Wave Generator

Each DAC channel has independent waveform, frequency, amplitude, offset, and output-enable controls. Choose sine, square, triangle, sawtooth, PPG, or DC. The waveform frequency range is **0.5 Hz-20 kHz**, in 0.1 Hz steps. Use the slider or type a precise frequency into the numeric field below it.

Enable the desired output explicitly. Waveform settings are saved, but the enabled state is never restored after a reboot. The animated graph is a **simulated preview**, not a measurement of the physical output. Its window is at most two seconds and shortens to four cycles at higher frequencies (200 microseconds at 20 kHz). Disabled outputs show a flat trace.

### Audio Output and Limits

Both DACs use the ESP32's I2S0/DMA driver at a configured **200,000 samples/s per channel**. Independent 32-bit phase accumulators and 1,024-entry lookup tables generate the waveforms. The GPIO2 heartbeat remains at 72 BPM regardless of the output frequency. This implementation targets the classic ESP32 and reserves I2S0 for the DACs.

The output path uses Espressif's [built-in DAC mode and I2S DMA driver](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32/api-reference/peripherals/i2s.html#configuring-i2s-to-use-internal-dac-for-analog-output), provided by the project's pinned Arduino/ESP-IDF toolchain.

At 20 kHz there are only **10 samples per cycle**. The 8-bit DAC produces stair steps and sampling images; square, triangle, sawtooth, and PPG shapes lose detail and can alias at high frequencies. The range is a software capability, not a measured bandwidth or distortion specification. Validate frequency, amplitude, distortion, and continuous streaming on your board before relying on audio performance. The internal ADC scope still samples at about 200 frames/s and cannot measure audio-range output correctly; use an external oscilloscope.

For listening, use a high-impedance **line input or powered amplifier**, with a shared ground and an appropriately designed attenuator and DC-blocking capacitor. A 1 kOhm series resistor can limit current but does not remove the DAC's DC offset or guarantee a suitable line level. Begin with a low amplitude and low amplifier volume. Do not drive a speaker or headphones directly from a DAC GPIO. Component choices and wiring must match the receiving input; see the [bench guide](docs/bench_verification_and_output_interface.md#audio-range-verification-issue-4).

Four 256-frame DMA buffers plus the producer block introduce up to roughly 6.4 ms of queued output at the nominal rate; controls and timed faults apply at block boundaries. `/api/state` reports `outputDriver`, `sampleRateHz`, `maxFrequencyHz`, and `streamWriteErrors`; the existing `timerRunning` and `timerTicks` fields now describe the DMA producer. `dacCodes` are the latest queued codes, not measured pin voltages. A failed or partial DMA write stops the stream and drives both DACs to zero. DMA buffers are cleared on underflow, but `streamWriteErrors` does not count every possible underflow; bench checks remain necessary, including during Wi-Fi traffic and settings saves.

### Oscilloscope Inputs

Select analog or digital PWM mode, then choose the input pins to display. Analog traces share a graph; PWM traces are reconstructed from captured edge timing, with frequency and duty-cycle readings.

| Function | GPIO pins | Notes |
| --- | --- | --- |
| DAC channel 1 | 25 | Built-in 8-bit DAC |
| DAC channel 2 | 26 | Built-in 8-bit DAC |
| Analog scope | 32, 33, 34, 35, 36, 39 | ADC1, raw 12-bit counts |
| Digital PWM capture | 4, 13, 14, 16, 17, 18, 19, 21, 22, 23, 27, 32, 33, 34, 35, 36, 39 | Digital inputs, not PWM outputs |

Analog inputs are sampled sequentially at about **200 frames/s per input**, with a **1.28-second buffer**. ADC2 is excluded because Wi-Fi is active. PWM capture is intended for low-frequency checks, not high-speed logic analysis.

## Electrical Limits

- Connect signal ground to ESP32 **GND**.
- Never apply a negative voltage or more than **3.3 V** to an input. ESP32 GPIOs are **not 5 V tolerant**.
- ADC readings are uncalibrated; full scale at the configured attenuation may be around 3.1 V and varies by chip.
- Use suitable input protection and buffering for external signals. Verify physical outputs with a scope or meter; the built-in DAC is uncalibrated.

See the [bench verification guide](docs/bench_verification_and_output_interface.md) for output measurements and interface considerations, and the [safety gap assessment](docs/Safety%20Gap%20assesment.md) for proposed standards and outstanding hardware work. These documents do not establish standards conformity.

## Terminal Commands

Open **PlatformIO Core CLI** from the PlatformIO sidebar and run these commands from `functiongenerator_web`:

```sh
# Compile firmware
pio run -e esp32dev

# Upload firmware to the connected board
pio run -e esp32dev -t upload

# Open Serial Monitor; Ctrl+C closes it
pio device monitor -b 115200
```

The unique setup-network name uses the station MAC shown on the splash screen and is stable for each device.
The webpage is embedded in the firmware. Changes to `include/wavegen_page.h` require rebuilding and uploading; no separate web server is needed.

## Troubleshooting

| Problem | What to check |
| --- | --- |
| `pio` is not found | Use PlatformIO Core CLI inside VS Code, where the extension's tools are available. |
| Board or serial port is missing | Try a known USB data cable and another USB port. Check whether your board's USB-to-serial chip requires a driver. |
| Upload reports that the port is busy | Close Serial Monitor and any other program using the board, then retry Upload. |
| Upload stays at `Connecting...` | Hold BOOT while connecting starts; release it when writing begins. |
| Setup network is missing | Check Serial Monitor: the ESP32 may already be connected to its saved Wi-Fi network. |
| Portal does not open | Stay connected to the ESP32 network named in Serial Monitor and open http://192.168.4.1 before the three-minute timeout. |
| Device webpage cannot be reached | Use the IP from the latest startup log and connect to the same router network. Guest-network or client isolation can block access. |
| Password prompt still appears | Upload the latest firmware, verify you are opening this ESP32's IP, and reload the page. |
| Output is flat | Enable the channel and check amplitude and waveform settings. DC and zero-amplitude signals are flat. Verify the actual DAC pin separately. |

## Developer Checks

Main files: `src/main.cpp` contains the firmware, `include/wavegen_page.h` contains the embedded webpage, and `platformio.ini` defines the board and dependencies.

## Device-information splash screen

Opening the web interface displays a dismissible splash with the program name,
full patch version, firmware compile date/time, Wi-Fi station MAC address, and
connected Wi-Fi SSID. Choose **Continue to generator** or press Escape to dismiss
it; **Device info** in the header opens it again. Viewing or dismissing the splash
does not enable outputs or change generator settings.

The information comes from the device’s `/api/state` response. Compile time
is embedded using `__DATE__` and `__TIME__` from the build machine, rather than the
browser clock or device boot time. Wi-Fi status refreshes with the existing
five-second state poll. Disconnected devices report **Not connected**; failed
requests show **Unavailable** and retry automatically. Older firmware without
the new metadata reports missing fields explicitly. The splash uses the same
firmware identifier as the connection badge and automatic version bumps.

## Version history
With Node.js installed, run:

```sh
node --test tests/web_regression.cjs
```

Entries are selected Git milestones, not numbered releases. Proposed standards remain awaiting approval.

The nine browser-logic regression tests cover generator controls, audio-frequency entry, preview frequency and animation, stale PWM polling, ordered PWM configuration requests, and splash metadata loading and error recovery. They use a mocked page/API and do not verify browser layout or physical signal accuracy. Use `git log --oneline` for revision history.

To also check LED pulse timing, install a host C++ compiler (`c++`, or set `CXX` to its executable) and run:

```sh
node --test tests/web_regression.cjs tests/heartbeat_regression.cjs tests/audio_regression.cjs
```

The heartbeat check verifies 72 pulses per minute, the PWM brightness range, and timing after long uptime. After uploading, confirm the GPIO2 LED pulses during Wi-Fi setup and with both outputs disabled, and that channel controls do not change its rate.

The audio check compiles the actual DMA producer against a mocked I2S sink and checks channel packing, independent frequencies through 20 kHz, DC and disabled outputs, timed fault behavior, and write-failure shutdown. Browser checks also cover 20 kHz entry and adaptive previews. These tests do not measure the hardware sample clock, CPU headroom, analog quality, or real DMA underflow behavior.

