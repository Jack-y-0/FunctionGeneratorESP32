#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <esp_timer.h>
#include <math.h>
#include "wavegen_page.h"

// Target: ESP32-S2 WROOM DevKit 1, as identified in the existing project.
// ESP32-S2 DAC1 and DAC2 are routed to GPIO17 and GPIO18 respectively.
static constexpr uint8_t DAC_PINS[2] = {17, 18};
static constexpr uint8_t BOOT_PIN = 0;
static constexpr uint32_t SAMPLE_RATE_HZ = 4000;
static constexpr uint32_t SAMPLE_PERIOD_US = 1000000UL / SAMPLE_RATE_HZ;
static constexpr float DAC_FULL_SCALE_V = 3.3f;
static constexpr float MIN_FREQUENCY_HZ = 0.5f;
static constexpr float MAX_FREQUENCY_HZ = 10.0f;

struct ChannelConfig {
  char waveform[12];
  float frequencyHz;
  float amplitudePercent; // peak-to-peak span as a percentage of DAC full scale
  float offsetPercent;    // center voltage as a percentage of DAC full scale
  bool enabled;
};

// Defaults preserve the current sketch's 72 BPM pulse and half-amplitude second output.
static ChannelConfig channelConfig[2] = {
  {"ppg", 1.2f, 90.0f, 50.0f, true},
  {"ppg", 1.2f, 45.0f, 50.0f, true}
};
static float phase[2] = {0.0f, 0.0f};
static portMUX_TYPE configMux = portMUX_INITIALIZER_UNLOCKED;
static esp_timer_handle_t sampleTimer = nullptr;
static WebServer server(80);

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

// Runs from the ESP timer task every 250 us; the two phase accumulators are independent.
static void onSampleTimer(void *) {
  // The existing sketch pauses its output while BOOT (GPIO0) is held low.
  if (digitalRead(BOOT_PIN) == LOW) return;

  ChannelConfig local[2];
  portENTER_CRITICAL(&configMux);
  local[0] = channelConfig[0];
  local[1] = channelConfig[1];
  portEXIT_CRITICAL(&configMux);

  for (uint8_t i = 0; i < 2; ++i) {
    if (!local[i].enabled) {
      dacWrite(DAC_PINS[i], 0);
      continue;
    }
    const float value = normalizedWaveform(local[i].waveform, phase[i]);
    dacWrite(DAC_PINS[i], sampleToDacCode(local[i], value));
    phase[i] += local[i].frequencyHz / static_cast<float>(SAMPLE_RATE_HZ);
    if (phase[i] >= 1.0f) phase[i] -= floorf(phase[i]);
  }
}

static void handleState() {
  ChannelConfig local[2];
  portENTER_CRITICAL(&configMux);
  local[0] = channelConfig[0];
  local[1] = channelConfig[1];
  portEXIT_CRITICAL(&configMux);

  String json = "{\"sampleRateHz\":" + String(SAMPLE_RATE_HZ) + ",\"channels\":[";
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
  server.send(200, "application/json", "{\"ok\":true}");
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
  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html; charset=utf-8", INDEX_HTML); });
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/channel", HTTP_POST, handleChannelUpdate);
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();
  Serial.println("Web server started.");
}

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_PIN, INPUT_PULLUP);
  delay(250);
  Serial.println();
  Serial.println("ESP32-S2 dual-channel waveform generator");
  Serial.println("DAC1 GPIO17 / DAC2 GPIO18");
  Serial.println("Serial: send ch1/ch2 then a0-a5 to select waveform.");

  dacWrite(DAC_PINS[0], 128);
  dacWrite(DAC_PINS[1], 128);

  WiFi.mode(WIFI_STA);
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);
  if (!wifiManager.autoConnect("ESP32-S2-WaveGen-Setup")) {
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
  delay(1);
}
