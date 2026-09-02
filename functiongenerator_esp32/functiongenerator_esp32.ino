// 26/08/2026.    dd/mm/yy
#define PROGRAMME_NAME "functiongenerator_esp32"
#define VERSION " V 0.0.1 "  // change ouput GPIO25 to GPIO02
#define MODEL_NAME "Model: functiongenerator_esp32"
#define DEVICE_UNDER_TEST "ESP32 S2 WRROM DevKit 1"
#define LICENSE "GNU Affero General Public License, version 3 "
#define ORIGIN "UK"

/*
  ESP32 Normal Heartbeat / PPG Simulator
  ---------------------------------------
  Arduino IDE

  Generates a repeatable, smooth PPG-style heartbeat waveform.

  Default:
    Heart rate: 72 BPM
    Waveform:   Normal sinus rhythm
    Output:     PWM on GPIO 02

  The waveform consists of:
    1. Baseline
    2. Rapid systolic rise
    3. Main pulse peak
    4. Slow diastolic decay
    5. Small dicrotic notch
    6. Return to baseline
*/

const int OUTPUT_PIN = 02;

// Heart rate
float BPM = 72.0;

// PWM settings
const int PWM_FREQUENCY = 5000;
const int PWM_RESOLUTION = 12;

// Maximum PWM value for 12-bit resolution
const int PWM_MAX = 4095;

// Pulse amplitude
// 0.0 = no pulse
// 1.0 = maximum amplitude
float AMPLITUDE = 0.80;

// Baseline output
float BASELINE = 0.05;

// Time resolution of waveform
const int SAMPLE_INTERVAL_MS = 2;


// ---------------------------------------------------------
// Gaussian function
// ---------------------------------------------------------

float gaussian(float x, float centre, float width)
{
  float exponent =
    -0.5 * pow((x - centre) / width, 2);

  return exp(exponent);
}


// ---------------------------------------------------------
// Generate one heartbeat waveform
// ---------------------------------------------------------

float heartbeatWaveform(float phase)
{
  /*
    phase = 0.0 → beginning of heartbeat
    phase = 1.0 → end of heartbeat
  */

  // Main systolic pulse
  float mainPulse =
    gaussian(phase, 0.18, 0.065);

  // Small dicrotic feature
  float dicroticWave =
    gaussian(phase, 0.43, 0.025);

  // Slight negative dip after the dicrotic wave
  float dicroticDip =
    gaussian(phase, 0.48, 0.030);

  // Combine components
  float pulse =
    mainPulse
    + 0.18 * dicroticWave
    - 0.07 * dicroticDip;

  // Make sure the signal doesn't become negative
  pulse = max(0.0f, pulse);

  // Apply amplitude
  pulse *= AMPLITUDE;

  // Add baseline
  float output =
    BASELINE + pulse;

  // Keep within valid range
  output = constrain(output, 0.0f, 1.0f);

  return output;
}

void splashserial() {
  Serial.println(F("==================================="));
  Serial.print(PROGRAMME_NAME);
  Serial.println(VERSION);
  Serial.println(MODEL_NAME);
  Serial.println(DEVICE_UNDER_TEST);
  Serial.print(F("Compiled at: "));
  Serial.println(F(__DATE__ " " __TIME__));
  Serial.println(LICENSE);
  Serial.println(F("==================================="));
  Serial.println();
}

// ---------------------------------------------------------
// Setup
// ---------------------------------------------------------

void setup()
{
  Serial.begin(115200);

  splashserial();

  // Configure PWM
  ledcAttach(OUTPUT_PIN, PWM_FREQUENCY, PWM_RESOLUTION);

  Serial.println();
  Serial.println("ESP32 Heartbeat Simulator");
  Serial.println("-------------------------");
  Serial.print("Heart rate: ");
  Serial.print(BPM);
  Serial.println(" BPM");

  Serial.println("Normal sinus rhythm");
  Serial.println("PPG-style waveform");
}


// ---------------------------------------------------------
// Main loop
// ---------------------------------------------------------

void loop()
{
  // Duration of one heartbeat in milliseconds
  float beatDuration =
    60000.0 / BPM;

  // Current position within heartbeat
  static unsigned long beatStartTime = millis();

  unsigned long currentTime = millis();

  // Time since beginning of current heartbeat
  float elapsed =
    currentTime - beatStartTime;

  // Start a new heartbeat
  if (elapsed >= beatDuration)
  {
    beatStartTime = currentTime;
    elapsed = 0;
  }

  // Convert time to phase 0 → 1
  float phase =
    elapsed / beatDuration;

  // Generate waveform
  float waveform =
    heartbeatWaveform(phase);

  // Convert waveform to PWM value
  int pwmValue =
    waveform * PWM_MAX;

  // Output waveform
  ledcWrite(OUTPUT_PIN, pwmValue);

  // Control waveform sample rate
  delay(SAMPLE_INTERVAL_MS);
}