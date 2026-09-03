// 26/08/2026.    dd/mm/yy
#define PROGRAMME_NAME "functiongenerator_esp32"
#define VERSION " V 0.0.3 "  // set GPIO25 to same as LED (GPIO02) and GPIO26 to 1/2 of LED, outputs a waveform similar to heartbeat on oscilloscope
#define MODEL_NAME "Model: functiongenerator_esp32"
#define DEVICE_UNDER_TEST "ESP32 S2 WRROM DevKit 1"
#define LICENSE "GNU Affero General Public License, version 3 "
#define ORIGIN "UK"

/*
  ============================================================
  ESP32 PPG HEARTBEAT SIGNAL SIMULATOR
  ============================================================

  OUTPUTS:

  GPIO 2  -> PWM -> Onboard LED
  GPIO 25 -> DAC -> Analogue PPG waveform

  Heart rate:
      72 BPM

  Waveform:
      Normal PPG-style pulse
      Systolic peak
      Diastolic decay
      Small dicrotic feature

  ============================================================
*/

#include <Arduino.h>


// ============================================================
// PIN CONFIGURATION
// ============================================================

const int LED_PIN = 2;       // Onboard LED
const int DAC_PIN = 25;      // DAC1 on classic ESP32


// ============================================================
// HEART RATE
// ============================================================

float BPM = 72.0;


// ============================================================
// WAVEFORM SETTINGS
// ============================================================

// DAC is 8-bit:
// 0    = 0 V
// 255  = approximately 3.3 V

// DC baseline
float BASELINE = 0.45;

// Pulsatile component
float AMPLITUDE = 0.12;


// ============================================================
// SAMPLING
// ============================================================

// Number of points used to describe one heartbeat
const int NUM_SAMPLES = 500;


// ============================================================
// GAUSSIAN FUNCTION
// ============================================================

float gaussian(float x, float centre, float width)
{
  return exp(
    -0.5 * pow((x - centre) / width, 2)
  );
}


// ============================================================
// HEARTBEAT WAVEFORM
// ============================================================

float heartbeatWaveform(float phase)
{
  /*
    phase:

    0.0 = beginning of heartbeat
    1.0 = end of heartbeat
  */


  // --------------------------------------------------------
  // MAIN SYSTOLIC PULSE
  // --------------------------------------------------------

  float mainPulse =
    gaussian(
      phase,
      0.18,
      0.065
    );


  // --------------------------------------------------------
  // DICROTIC WAVE
  // --------------------------------------------------------

  float dicroticWave =
    gaussian(
      phase,
      0.43,
      0.025
    );


  // --------------------------------------------------------
  // SMALL DIP AFTER DICROTIC WAVE
  // --------------------------------------------------------

  float dicroticDip =
    gaussian(
      phase,
      0.48,
      0.030
    );


  // --------------------------------------------------------
  // COMBINE WAVEFORM
  // --------------------------------------------------------

  float pulse =
      mainPulse
    + 0.18 * dicroticWave
    - 0.07 * dicroticDip;


  // Prevent negative values
  pulse = max(0.0f, pulse);


  // Apply amplitude
  pulse *= AMPLITUDE;


  // Add DC baseline
  float output =
    BASELINE + pulse;


  // Limit to 0–1
  output =
    constrain(output, 0.0f, 1.0f);


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

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  // splashserial();

  dacWrite(25, 128); // Sets GPIO25 to midrange
  dacWrite(26, 64); // Sets GPIO26 to quarter-range   

  // --------------------------------------------------------
  // GPIO 2 PWM
  // --------------------------------------------------------

  // 5 kHz PWM
  // 8-bit resolution
  ledcAttach(
    LED_PIN,
    5000,
    8
  );


  // --------------------------------------------------------
  // INFORMATION
  // --------------------------------------------------------

  Serial.println();
  Serial.println("======================================");
  Serial.println("ESP32 PPG HEARTBEAT SIMULATOR");
  Serial.println("======================================");

  Serial.print("Heart rate: ");
  Serial.print(BPM);
  Serial.println(" BPM");

  Serial.print("Beat duration: ");
  Serial.print(60000.0 / BPM);
  Serial.println(" ms");

  Serial.println();

  Serial.println("GPIO 2  -> LED / PWM");
  Serial.println("GPIO 25 -> Analogue DAC");

  Serial.println("======================================");
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
  // --------------------------------------------------------
  // Calculate heartbeat duration
  // --------------------------------------------------------

  float beatDuration = 60000.0 / BPM;

  // --------------------------------------------------------
  // Time between waveform samples
  // --------------------------------------------------------

  float sampleTime = beatDuration / NUM_SAMPLES;

  // --------------------------------------------------------
  // Generate one heartbeat
  // --------------------------------------------------------

  for (int i = 0; i < NUM_SAMPLES; i++)
  {
    // Position within heartbeat
    float phase =
      (float)i / (NUM_SAMPLES - 1);

    // Generate waveform
    float waveform =
      heartbeatWaveform(phase);


    // ======================================================
    // GPIO 2 — LED
    // ======================================================

    // Make the LED response MUCH more obvious.
    //
    // Instead of using the small 0.45–0.57 waveform range,
    // map it onto a much larger brightness range.

    int ledBrightness =
      map(
        (int)(waveform * 1000),
        450,
        570,
        10,
        255
      );

    ledBrightness =
      constrain(
        ledBrightness,
        10,
        255
      );

    ledcWrite(
      LED_PIN,
      ledBrightness
    );

    dacWrite(25, ledBrightness); // Sets GPIO25 to same as LED
    dacWrite(26, ledBrightness/2); // Sets GPIO26 to 1/2 of LED


    // ------------------------------------------------------
    // Timing
    // ------------------------------------------------------

    delayMicroseconds(
      (unsigned long)(sampleTime * 1000)
    );
  }
}