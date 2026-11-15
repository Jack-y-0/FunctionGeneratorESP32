#pragma once

#include <stdint.h>
#include <math.h>

static constexpr uint32_t DAC_SAMPLE_RATE_HZ = 200000;
static constexpr float DAC_MAX_FREQUENCY_HZ = 20000.0f;
static constexpr uint16_t DAC_TABLE_SIZE = 1024;
static constexpr uint16_t DAC_BLOCK_FRAMES = 256;
static_assert(DAC_TABLE_SIZE == 1024, "Phase indexing uses the upper ten bits");

struct DacPhase {
  uint32_t value = 0;
  uint32_t step = 0;

  void setFrequency(float hz) {
    step = static_cast<uint32_t>(llround(static_cast<double>(hz) * 4294967296.0 / DAC_SAMPLE_RATE_HZ));
  }

  uint16_t index() const { return value >> 22; }

  bool advance() {
    const uint32_t previous = value;
    value += step;
    return value < previous;
  }
};

inline uint32_t packDacFrame(uint8_t channel1, uint8_t channel2) {
  // 16-bit stereo slots: left (DAC2/GPIO26), then right (DAC1/GPIO25).
  // The DAC consumes only each slot's upper eight bits.
  return (static_cast<uint32_t>(channel1) << 24) | (static_cast<uint32_t>(channel2) << 8);
}
