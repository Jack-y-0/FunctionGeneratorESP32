#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <math.h>
#include "wavegen_page.h"

// Pin selection follows the PlatformIO target: classic ESP32 uses GPIO25/26;
// ESP32-S2 uses GPIO17/18 for its built-in DAC channels.
#if defined(CONFIG_IDF_TARGET_ESP32S2)
static constexpr const char *FIRMWARE_BUILD_ID = "channel-debug-2026-11-07";
static constexpr const char *BOARD_TARGET_NAME = "ESP32-S2";
static constexpr uint8_t DAC_PINS[2] = {17, 18};
#elif defined(CONFIG_IDF_TARGET_ESP32)
static constexpr const char *BOARD_TARGET_NAME = "ESP32";
static constexpr uint8_t DAC_PINS[2] = {25, 26};
#else
#error "This project supports classic ESP32 and ESP32-S2 targets only."
#endif
static constexpr uint8_t BOOT_PIN = 0;
static constexpr uint32_t SAMPLE_RATE_HZ = 4000;
static constexpr uint32_t SAMPLE_PERIOD_US = 1000000UL / SAMPLE_RATE_HZ;
static constexpr float DAC_FULL_SCALE_V = 3.3f;
static constexpr float MIN_FREQUENCY_HZ = 0.5f;
static constexpr float MAX_FREQUENCY_HZ = 10.0f;

enum FaultMode : uint8_t { FAULT_NONE, FAULT_MISSING_PULSE, FAULT_REDUCED_AMPLITUDE, FAULT_BASELINE_SHIFT, FAULT_DROPOUT };
struct FaultConfig { FaultMode mode; uint64_t expiresAtUs; uint32_t beatsSinceStart; bool active; bool skipCurrentBeat; };
struct EventRecord { uint32_t uptimeMs; uint8_t channel; char scenario[20]; char action[8]; };

struct ChannelConfig {
  char waveform[12];
  float frequencyHz;
  float amplitudePercent; // peak-to-peak span as a percentage of DAC full scale
  float offsetPercent;    // center voltage as a percentage of DAC full scale
  bool enabled;
};

// Start both outputs disabled; enable a channel explicitly from the webpage.
static ChannelConfig channelConfig[2] = {
  {"ppg", 1.2f, 90.0f, 50.0f, false},
  {"ppg", 1.2f, 45.0f, 50.0f, false}
};
static float phase[2] = {0.0f, 0.0f};
static FaultConfig faultConfig[2] = {};
static EventRecord eventLog[24] = {};
static uint8_t eventLogNext = 0;
static uint8_t eventLogCount = 0;
static portMUX_TYPE configMux = portMUX_INITIALIZER_UNLOCKED;
static esp_timer_handle_t sampleTimer = nullptr;
static WebServer server(80);
static Preferences settingsStore;
static bool settingsDirty = false;
static uint32_t lastSettingsChangeMs = 0;
static String webPassword;
static String setupApPassword;

static const char *waveformForCommand(const String &command) {
  if (command == "a0") return "sine";
  if (command == "a1") return "square";
  if (command == "a2") return "triangle";
  if (command == "a3") return "sawtooth";
  if (command == "a4") return "ppg";
  if (command == "a5") return "dc";
  return nullptr;
}

static bool validWaveform(const String &waveform) {
  return waveform == "sine" || waveform == "square" ||
         waveform == "triangle" || waveform == "sawtooth" || waveform == "ppg" || waveform == "dc";
}

static String makeRandomPassword();

static void loadChannelSettings() {
  settingsStore.begin("wavegen", false);
  webPassword = settingsStore.getString("webpass", "");
  if (webPassword.length() < 12) {
    webPassword = makeRandomPassword();
    settingsStore.putString("webpass", webPassword);
  }
  Serial.print("Web login: username=admin password=");
  Serial.println(webPassword);
  for (uint8_t i = 0; i < 2; ++i) {
    const String prefix = "ch" + String(i) + "_";
    const String savedWaveform = settingsStore.getString((prefix + "wave").c_str(), channelConfig[i].waveform);
    if (validWaveform(savedWaveform)) {
      strlcpy(channelConfig[i].waveform, savedWaveform.c_str(), sizeof(channelConfig[i].waveform));
    }
    channelConfig[i].frequencyHz = constrain(settingsStore.getFloat((prefix + "freq").c_str(), channelConfig[i].frequencyHz), MIN_FREQUENCY_HZ, MAX_FREQUENCY_HZ);
    channelConfig[i].amplitudePercent = constrain(settingsStore.getFloat((prefix + "amp").c_str(), channelConfig[i].amplitudePercent), 0.0f, 100.0f);
    channelConfig[i].offsetPercent = constrain(settingsStore.getFloat((prefix + "offset").c_str(), channelConfig[i].offsetPercent), 0.0f, 100.0f);
    // Never restore the enabled flag: outputs remain off after every reboot.
    channelConfig[i].enabled = false;
  }
}

static void saveChannelSettingsIfDue() {
  if (!settingsDirty || millis() - lastSettingsChangeMs < 1500) return;
  for (uint8_t i = 0; i < 2; ++i) {
    const String prefix = "ch" + String(i) + "_";
    settingsStore.putString((prefix + "wave").c_str(), channelConfig[i].waveform);
    settingsStore.putFloat((prefix + "freq").c_str(), channelConfig[i].frequencyHz);
    settingsStore.putFloat((prefix + "amp").c_str(), channelConfig[i].amplitudePercent);
    settingsStore.putFloat((prefix + "offset").c_str(), channelConfig[i].offsetPercent);
  }
  settingsDirty = false;
}

static void markSettingsDirty() {
  settingsDirty = true;
  lastSettingsChangeMs = millis();
}

static String makeRandomPassword() {
  char password[17];
  snprintf(password, sizeof(password), "%08lx%08lx",
           static_cast<unsigned long>(esp_random()),
           static_cast<unsigned long>(esp_random()));
  return String(password);
}

static bool requireWebAuth() {
  if (server.authenticate("admin", webPassword.c_str())) return true;
  server.requestAuthentication(BASIC_AUTH, "ESP32_MUXSIGSIM");
  return false;
}

static float gaussian(float x, float centre, float width) {
  const float z = (x - centre) / width;
  return expf(-0.5f * z * z);
}

// Port of the existing project's systolic peak, dicrotic feature, and small dip.
static float ppgWaveform(float p) {
  const float pulse = fmaxf(0.0f,
      gaussian(p, 0.18f, 0.065f)
    + 0.18f * gaussian(p, 0.43f, 0.025f)
    - 0.07f * gaussian(p, 0.48f, 0.030f));
  return 0.05f + 0.95f * fminf(1.0f, pulse);
}

static float normalizedWaveform(const char *waveform, float p) {
  if (strcmp(waveform, "square") == 0) return p < 0.5f ? 1.0f : 0.0f;
  if (strcmp(waveform, "triangle") == 0) return 1.0f - fabsf(2.0f * p - 1.0f);
  if (strcmp(waveform, "sawtooth") == 0) return p;
  if (strcmp(waveform, "ppg") == 0) return ppgWaveform(p);
  if (strcmp(waveform, "dc") == 0) return 0.5f;
  return 0.5f + 0.5f * sinf(2.0f * PI * p);
}

static uint8_t sampleToDacCode(const ChannelConfig &config, float wave) {
  const float center = (config.offsetPercent / 100.0f) * DAC_FULL_SCALE_V;
  const float peakToPeak = (config.amplitudePercent / 100.0f) * DAC_FULL_SCALE_V;
  float volts = center + peakToPeak * (wave - 0.5f);
  volts = constrain(volts, 0.0f, DAC_FULL_SCALE_V);
  return static_cast<uint8_t>(lroundf((volts / DAC_FULL_SCALE_V) * 255.0f));
}

// Runs from the ESP timer task every 250 us and applies an active timed fault.
static void onSampleTimer(void *) {
  if (digitalRead(BOOT_PIN) == LOW) return;
  ChannelConfig local[2];
  FaultConfig faults[2];
  portENTER_CRITICAL(&configMux);
  local[0] = channelConfig[0]; local[1] = channelConfig[1];
  faults[0] = faultConfig[0]; faults[1] = faultConfig[1];
  portEXIT_CRITICAL(&configMux);
  const uint64_t nowUs = esp_timer_get_time();
  for (uint8_t i = 0; i < 2; ++i) {
    if (!local[i].enabled) { dacWrite(DAC_PINS[i], 0); continue; }
    const bool active = faults[i].active && nowUs < faults[i].expiresAtUs;
    if (active && faults[i].mode == FAULT_DROPOUT) {
      dacWrite(DAC_PINS[i], 0);
    } else {
      ChannelConfig effective = local[i];
      if (active && faults[i].mode == FAULT_REDUCED_AMPLITUDE) effective.amplitudePercent *= 0.5f;
      if (active && faults[i].mode == FAULT_BASELINE_SHIFT) effective.offsetPercent = fminf(100.0f, effective.offsetPercent + 10.0f);
      float value = normalizedWaveform(effective.waveform, phase[i]);
      if (active && faults[i].mode == FAULT_MISSING_PULSE && strcmp(effective.waveform, "ppg") == 0 && faults[i].skipCurrentBeat) value = 0.05f;
      dacWrite(DAC_PINS[i], sampleToDacCode(effective, value));
    }
    phase[i] += local[i].frequencyHz / static_cast<float>(SAMPLE_RATE_HZ);
    if (phase[i] >= 1.0f) {
      phase[i] -= floorf(phase[i]);
      if (active && faults[i].mode == FAULT_MISSING_PULSE) {
        portENTER_CRITICAL(&configMux);
        if (faultConfig[i].active && faultConfig[i].mode == FAULT_MISSING_PULSE) {
          ++faultConfig[i].beatsSinceStart;
          faultConfig[i].skipCurrentBeat = (faultConfig[i].beatsSinceStart % 3U) == 0U;
        }
        portEXIT_CRITICAL(&configMux);
      }
    }
  }
}
static void handleState() {
  ChannelConfig local[2];
  portENTER_CRITICAL(&configMux);
  local[0] = channelConfig[0];
  local[1] = channelConfig[1];
  portEXIT_CRITICAL(&configMux);

  String json = "{\"sampleRateHz\":" + String(SAMPLE_RATE_HZ) + ",\"pins\":[" + String(DAC_PINS[0]) + "," + String(DAC_PINS[1]) + "],\"channels\":[";
  for (uint8_t i = 0; i < 2; ++i) {
    if (i) json += ',';
    json += "{\"waveform\":\"" + String(local[i].waveform) +
            "\",\"frequency\":" + String(local[i].frequencyHz, 1) +
            ",\"amplitude\":" + String(local[i].amplitudePercent, 0) +
            ",\"offset\":" + String(local[i].offsetPercent, 0) +
            ",\"enabled\":" + String(local[i].enabled ? "true" : "false") + "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

static void handleChannelUpdate() {
  if (!server.hasArg("channel")) {
    server.send(400, "application/json", "{\"error\":\"Missing channel\"}");
    return;
  }
  const int index = server.arg("channel").toInt();
  if (index < 0 || index > 1) {
    server.send(400, "application/json", "{\"error\":\"Channel must be 0 or 1\"}");
    return;
  }

  ChannelConfig updated;
  portENTER_CRITICAL(&configMux);
  updated = channelConfig[index];
  portEXIT_CRITICAL(&configMux);

  if (server.hasArg("command")) {
    const char *wave = waveformForCommand(server.arg("command"));
    if (wave == nullptr) {
      server.send(400, "application/json", R"json({"error":"Use command a0 through a5"})json");
      return;
    }
    strlcpy(updated.waveform, wave, sizeof(updated.waveform));
  } else if (server.hasArg("waveform")) {
    const String wave = server.arg("waveform");
    if (!validWaveform(wave)) {
      server.send(400, "application/json", R"json({"error":"Unsupported waveform"})json");
      return;
    }
    strlcpy(updated.waveform, wave.c_str(), sizeof(updated.waveform));
  }
  if (server.hasArg("frequency")) {
    const float frequency = server.arg("frequency").toFloat();
    if (!isfinite(frequency) || frequency < MIN_FREQUENCY_HZ || frequency > MAX_FREQUENCY_HZ) {
      server.send(400, "application/json", "{\"error\":\"Frequency must be 0.5 to 10 Hz\"}");
      return;
    }
    updated.frequencyHz = frequency;
  }
  if (server.hasArg("amplitude")) {
    const float amplitude = server.arg("amplitude").toFloat();
    if (!isfinite(amplitude) || amplitude < 0.0f || amplitude > 100.0f) {
      server.send(400, "application/json", "{\"error\":\"Amplitude must be 0 to 100 percent\"}");
      return;
    }
    updated.amplitudePercent = amplitude;
  }
  if (server.hasArg("offset")) {
    const float offset = server.arg("offset").toFloat();
    if (!isfinite(offset) || offset < 0.0f || offset > 100.0f) {
      server.send(400, "application/json", "{\"error\":\"Offset must be 0 to 100 percent\"}");
      return;
    }
    updated.offsetPercent = offset;
  }
  if (server.hasArg("enabled")) updated.enabled = server.arg("enabled") == "1";

  portENTER_CRITICAL(&configMux);
  channelConfig[index] = updated;
  portEXIT_CRITICAL(&configMux);
  markSettingsDirty();

  ChannelConfig applied;
  portENTER_CRITICAL(&configMux);
  applied = channelConfig[index];
  portEXIT_CRITICAL(&configMux);
  Serial.printf("Web update CH%d: enabled=%d waveform=%s freq=%.1fHz amp=%.0f%% offset=%.0f%%\\n",
                index + 1, applied.enabled, applied.waveform, applied.frequencyHz,
                applied.amplitudePercent, applied.offsetPercent);
  String response = "{\\"ok\\":true,\\"channel\\":" + String(index) +
                    ",\\"waveform\\":\\"" + String(applied.waveform) +
                    "\\",\\"frequency\\":" + String(applied.frequencyHz, 1) +
                    ",\\"amplitude\\":" + String(applied.amplitudePercent, 0) +
                    ",\\"offset\\":" + String(applied.offsetPercent, 0) +
                    ",\\"enabled\\":" + String(applied.enabled ? "true" : "false") + "}";
  server.send(200, "application/json", response);
}

static const char *faultName(FaultMode mode) {
  switch (mode) {
    case FAULT_MISSING_PULSE: return "missing_pulse";
    case FAULT_REDUCED_AMPLITUDE: return "reduced_amplitude";
    case FAULT_BASELINE_SHIFT: return "baseline_shift";
    case FAULT_DROPOUT: return "dropout";
    default: return "none";
  }
}

static void recordFaultEvent(uint8_t channel, FaultMode mode, const char *action) {
  EventRecord &event = eventLog[eventLogNext];
  event.uptimeMs = millis();
  event.channel = channel + 1;
  strlcpy(event.scenario, faultName(mode), sizeof(event.scenario));
  strlcpy(event.action, action, sizeof(event.action));
  eventLogNext = (eventLogNext + 1) % 24;
  if (eventLogCount < 24) ++eventLogCount;
}

static void handleFaultUpdate() {
  if (!server.hasArg("channel") || !server.hasArg("mode")) {
    server.send(400, "application/json", "{\"error\":\"Missing channel or mode\"}");
    return;
  }
  const int index = server.arg("channel").toInt();
  if (index < 0 || index > 1) {
    server.send(400, "application/json", "{\"error\":\"Channel must be 0 or 1\"}");
    return;
  }
  const String requested = server.arg("mode");
  FaultMode mode = FAULT_NONE;
  if (requested == "missing_pulse") mode = FAULT_MISSING_PULSE;
  else if (requested == "reduced_amplitude") mode = FAULT_REDUCED_AMPLITUDE;
  else if (requested == "baseline_shift") mode = FAULT_BASELINE_SHIFT;
  else if (requested == "dropout") mode = FAULT_DROPOUT;
  else if (requested != "none") {
    server.send(400, "application/json", "{\"error\":\"Unsupported fault mode\"}");
    return;
  }
  uint32_t durationSeconds = 0;
  if (mode != FAULT_NONE) {
    portENTER_CRITICAL(&configMux);
    const bool outputEnabled = channelConfig[index].enabled;
    portEXIT_CRITICAL(&configMux);
    if (!outputEnabled) {
      server.send(409, "application/json", "{\"error\":\"Enable this output before starting a fault scenario\"}");
      return;
    }
    durationSeconds = server.arg("duration").toInt();
    if (durationSeconds < 1 || durationSeconds > 600) {
      server.send(400, "application/json", "{\"error\":\"Duration must be 1 to 600 seconds\"}");
      return;
    }
    if (mode == FAULT_MISSING_PULSE) {
      portENTER_CRITICAL(&configMux);
      const bool isPpg = strcmp(channelConfig[index].waveform, "ppg") == 0;
      portEXIT_CRITICAL(&configMux);
      if (!isPpg) {
        server.send(400, "application/json", "{\"error\":\"Missing pulse requires PPG waveform\"}");
        return;
      }
    }
  }
  portENTER_CRITICAL(&configMux);
  faultConfig[index].mode = mode;
  faultConfig[index].active = mode != FAULT_NONE;
  faultConfig[index].expiresAtUs = esp_timer_get_time() + static_cast<uint64_t>(durationSeconds) * 1000000ULL;
  faultConfig[index].beatsSinceStart = 0;
  faultConfig[index].skipCurrentBeat = false;
  portEXIT_CRITICAL(&configMux);
  recordFaultEvent(index, mode, mode == FAULT_NONE ? "stopped" : "started");
  server.send(200, "application/json", "{\"ok\":true}");
}

static void handleFaultEvents() {
  String json = "{\"events\":[";
  const uint8_t first = (eventLogNext + 24 - eventLogCount) % 24;
  for (uint8_t i = 0; i < eventLogCount; ++i) {
    if (i) json += ',';
    const EventRecord &event = eventLog[(first + i) % 24];
    json += "{\"uptimeMs\":";
    json += String(event.uptimeMs);
    json += ",\"channel\":";
    json += String(event.channel);
    json += ",\"scenario\":\"";
    json += event.scenario;
    json += "\",\"action\":\"";
    json += event.action;
    json += "\"}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

static void serviceFaultTimers() {
  const uint64_t nowUs = esp_timer_get_time();
  for (uint8_t i = 0; i < 2; ++i) {
    FaultMode expiredMode = FAULT_NONE;
    portENTER_CRITICAL(&configMux);
    if (faultConfig[i].active && nowUs >= faultConfig[i].expiresAtUs) {
      expiredMode = faultConfig[i].mode;
      faultConfig[i].active = false;
      faultConfig[i].mode = FAULT_NONE;
    }
    portEXIT_CRITICAL(&configMux);
    if (expiredMode != FAULT_NONE) recordFaultEvent(i, expiredMode, "ended");
  }
}

static uint8_t serialTargetChannel = 0;
static char serialBuffer[24];
static size_t serialBufferLength = 0;

static void processSerialCommand() {
  serialBuffer[serialBufferLength] = '\0';
  String command(serialBuffer);
  command.trim();
  command.toLowerCase();
  serialBufferLength = 0;
  if (command.length() == 0) return;

  if (command == "ch1" || command == "ch2") {
    serialTargetChannel = (command == "ch1") ? 0 : 1;
    Serial.printf("Serial target: Channel %u (GPIO%u)\n", serialTargetChannel + 1, DAC_PINS[serialTargetChannel]);
    return;
  }

  const char *wave = waveformForCommand(command);
  if (wave == nullptr) {
    Serial.println("Commands: ch1/ch2 select output; a0=sine, a1=square, a2=triangle, a3=sawtooth, a4=PPG, a5=DC");
    return;
  }
  ChannelConfig updated;
  portENTER_CRITICAL(&configMux);
  updated = channelConfig[serialTargetChannel];
  portEXIT_CRITICAL(&configMux);
  strlcpy(updated.waveform, wave, sizeof(updated.waveform));
  portENTER_CRITICAL(&configMux);
  channelConfig[serialTargetChannel] = updated;
  portEXIT_CRITICAL(&configMux);
  markSettingsDirty();
  Serial.printf("Channel %u waveform: %s\n", serialTargetChannel + 1, wave);
}

static void serviceSerial() {
  while (Serial.available() > 0) {
    const char ch = static_cast<char>(Serial.read());
    if (ch == '\r' || ch == '\n') {
      processSerialCommand();
    } else if (serialBufferLength < sizeof(serialBuffer) - 1) {
      serialBuffer[serialBufferLength++] = ch;
    }
  }
}

static void beginWebServer() {
  server.on("/", HTTP_GET, []() { if (!requireWebAuth()) return; server.send_P(200, "text/html; charset=utf-8", INDEX_HTML); });
  server.on("/api/state", HTTP_GET, []() { if (!requireWebAuth()) return; handleState(); });
  server.on("/api/channel", HTTP_POST, []() { if (!requireWebAuth()) return; handleChannelUpdate(); });
  server.on("/api/fault", HTTP_POST, []() { if (!requireWebAuth()) return; handleFaultUpdate(); });
  server.on("/api/events", HTTP_GET, []() { if (!requireWebAuth()) return; handleFaultEvents(); });
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();
  Serial.println("Web server started.");
}

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_PIN, INPUT_PULLUP);
  delay(250);
  Serial.println();
  Serial.print(BOARD_TARGET_NAME);
  Serial.println(" dual-channel waveform generator");
  Serial.print("DAC1 GPIO");
  Serial.print(DAC_PINS[0]);
  Serial.print(" / DAC2 GPIO");
  Serial.println(DAC_PINS[1]);
  Serial.println("Serial: send ch1/ch2 then a0-a5 to select waveform.");
  loadChannelSettings();
  Serial.println("Saved waveform settings loaded; outputs remain disabled.");

  // Hold both outputs at zero until the user enables them from the webpage.
  dacWrite(DAC_PINS[0], 0);
  dacWrite(DAC_PINS[1], 0);

  WiFi.mode(WIFI_STA);
  setupApPassword = makeRandomPassword();
  Serial.print("Setup AP: ESP32_MUXSIGSIM password=");
  Serial.println(setupApPassword);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);
  if (!wifiManager.autoConnect("ESP32_MUXSIGSIM", setupApPassword.c_str())) {
    Serial.println("Wi-Fi setup timed out. Restarting; reconnect to the setup network to try again.");
    delay(500);
    ESP.restart();
  }
  Serial.print("Connected to Wi-Fi. Open http://");
  Serial.println(WiFi.localIP());

  beginWebServer();

  esp_timer_create_args_t timerArgs = {};
  timerArgs.callback = &onSampleTimer;
  timerArgs.name = "wave_sample";
  timerArgs.dispatch_method = ESP_TIMER_TASK;
  const esp_err_t createResult = esp_timer_create(&timerArgs, &sampleTimer);
  if (createResult != ESP_OK) {
    Serial.printf("Could not create sample timer (error %d).\n", createResult);
    return;
  }
  const esp_err_t startResult = esp_timer_start_periodic(sampleTimer, SAMPLE_PERIOD_US);
  if (startResult != ESP_OK) Serial.printf("Could not start sample timer (error %d).\n", startResult);
}

void loop() {
  serviceSerial();
  server.handleClient();
  saveChannelSettingsIfDue();
  serviceFaultTimers();
  delay(1);
}
