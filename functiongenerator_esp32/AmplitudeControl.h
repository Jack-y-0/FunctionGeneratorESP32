#pragma once

#include <stdint.h>

// Accepts a0..a99 followed by Newline or CRLF.
// Does not wait for a complete command.
class AmplitudeCommandParser {
 public:
  enum Result { Pending, Accepted, Invalid };

  Result feed(char c, uint8_t &level) {
    if (c == '\n') {
      unsigned int length = length_;

      // Allow CRLF as well as LF.
      if (length > 0 && buffer_[length - 1] == '\r') {
        --length;
      }

      bool valid =
          !overflow_ &&
          (length == 2 || length == 3) &&
          buffer_[0] == 'a';

      uint8_t value = 0;

      for (unsigned int i = 1; valid && i < length; ++i) {
        if (buffer_[i] < '0' || buffer_[i] > '9') {
          valid = false;
        } else {
          value = value * 10 + (buffer_[i] - '0');
        }
      }

      length_ = 0;
      overflow_ = false;

      if (!valid) return Invalid;

      level = value;
      return Accepted;
    }

    if (length_ < sizeof(buffer_)) {
      buffer_[length_++] = c;
    } else {
      // Reject an overlong command when Newline arrives.
      overflow_ = true;
    }

    return Pending;
  }

 private:
  char buffer_[4] = {};
  unsigned int length_ = 0;
  bool overflow_ = false;
};

// Scale the pulse while preserving its baseline.
inline uint8_t scaleDacAmplitude(uint8_t fullScale, uint8_t level) {
  const uint8_t baseline = 10;

  if (level > 99) level = 99;
  if (fullScale < baseline) fullScale = baseline;

  // Integer arithmetic, rounded to the nearest DAC code.
  return baseline + ((fullScale - baseline) * level + 49) / 99;
}