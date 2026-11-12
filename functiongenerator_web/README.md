# ESP32 Function Generator and Scope

Browser-controlled waveform generator and input scope for the **classic ESP32 DevKit** (`esp32dev`). Includes two independent DAC outputs, six analog inputs, and 17 selectable PWM capture inputs. Both outputs start disabled after every reboot.

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

1. On first startup, or if the saved network cannot be reached, connect your phone or computer to **ESP32_MUXSIGSIM**. This setup network has **no password**. Stay connected if your device reports that it has no internet access.
2. Open the WiFiManager portal when prompted. If it does not open automatically, visit [http://192.168.4.1](http://192.168.4.1).
3. Choose your 2.4 GHz router network, enter **the router's Wi-Fi password** if required, and save. The setup portal times out after three minutes; the ESP32 then restarts so you can try again.
4. Reconnect your phone or computer to the same router network as the ESP32.
5. In VS Code's Serial Monitor, find `Connected to Wi-Fi. Open http://` followed by the device IP address. Open that address in your browser.

The webpage and API require **no username or password**. Anyone on the connected network can access the controls. Saved Wi-Fi credentials are reused on later boots, so the setup network normally disappears after connection.

## Use the Controls

### Wave Generator

Each DAC channel has independent waveform, frequency, amplitude, offset, and output-enable controls. Choose sine, square, triangle, sawtooth, PPG, or DC. The waveform frequency range is **0.5-10 Hz**.

Enable the desired output explicitly. Waveform settings are saved, but the enabled state is never restored after a reboot. The animated graph is a **simulated preview over a two-second window**, not a measurement of the physical output. Disabled outputs show a flat trace.

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

The webpage is embedded in the firmware. Changes to `include/wavegen_page.h` require rebuilding and uploading; no separate web server is needed.

## Troubleshooting

| Problem | What to check |
| --- | --- |
| `pio` is not found | Use PlatformIO Core CLI inside VS Code, where the extension's tools are available. |
| Board or serial port is missing | Try a known USB data cable and another USB port. Check whether your board's USB-to-serial chip requires a driver. |
| Upload reports that the port is busy | Close Serial Monitor and any other program using the board, then retry Upload. |
| Upload stays at `Connecting...` | Hold BOOT while connecting starts; release it when writing begins. |
| Setup network is missing | Check Serial Monitor: the ESP32 may already be connected to its saved Wi-Fi network. |
| Portal does not open | Stay connected to ESP32_MUXSIGSIM and open http://192.168.4.1 before the three-minute timeout. |
| Device webpage cannot be reached | Use the IP from the latest startup log and connect to the same router network. Guest-network or client isolation can block access. |
| Password prompt still appears | Upload the latest firmware, verify you are opening this ESP32's IP, and reload the page. |
| Output is flat | Enable the channel and check amplitude and waveform settings. DC and zero-amplitude signals are flat. Verify the actual DAC pin separately. |

## Developer Checks

Main files: `src/main.cpp` contains the firmware, `include/wavegen_page.h` contains the embedded webpage, and `platformio.ini` defines the board and dependencies.

With Node.js installed, run:

```sh
node --test tests/web_regression.cjs
```

The four regression tests cover generator controls, preview frequency and animation, stale PWM polling, and ordered PWM configuration requests. They use a mocked page/API and do not verify browser layout or physical signal accuracy. Use `git log --oneline` for revision history.
