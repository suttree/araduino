#pragma once
#include <stdint.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#define ARA_FLASH PROGMEM
#else
#define ARA_FLASH
#endif
#include "SineTable.h"

// Shared by the Uno sketch and the desktop renderer. No allocation or floats.
// Call control() at 256 Hz and audio() at 16384 Hz.
namespace ara {
class BirdVoice {
 public:
  enum Profile : uint8_t { Soft, Garden, Lively };
  // 1587/1000 raises the register by approximately eight semitones.
  explicit BirdVoice(uint32_t seed = 1, uint16_t pitchScale = 1587)
      : rng(seed ? seed : 1), pitchScale(pitchScale) {}
  bool busy() const { return active; }
  void trigger(Profile profile) {
    if (active) return;
    mode = profile;
    count = profile == Soft ? 4 : 5;
    const int base = 1850 + pick(650);
    for (uint8_t i = 0; i < count; ++i) {
      Syllable &s = motif[i];
      s.start = base + pick(650) - 220;
      s.end = s.start + pick(650) - 325;
      s.bend = (i % 2 ? -1 : 1) * (180 + pick(320));
      s.ticks = profile == Soft ? 46 + pick(35) : 23 + pick(35);
      s.gap = 13 + pick(24);
      s.level = 100 + pick(24);
      s.trill = profile == Lively && i % 2 == 1;
      if (s.trill) s.ticks += 28;
    }
    index = 0;
    age = 0;
    repeatShift = pick(61) - 30;
    active = true;
  }
  void control() {
    if (!active) { ramp(0, increment); return; }
    const Syllable &s = motif[index % count];
    if (age < s.ticks) {
      const uint16_t t = uint32_t(age) * 255 / (s.ticks - 1);
      // Keep the fraction until after multiplication by the bend depth.
      int32_t hz = s.start + int32_t(s.end - s.start) * t / 255;
      hz += int32_t(s.bend) * 4 * t * (255 - t) / 65025;
      if (index >= count) hz += repeatShift;
      uint8_t level = s.level;
      const uint8_t attack = mode == Soft ? 7 : 4;
      const uint8_t release = mode == Soft ? 12 : 7;
      if (age < attack) level = uint16_t(level) * age / attack;
      if (s.ticks - 1 - age < release)
        level = uint16_t(level) * (s.ticks - 1 - age) / release;
      if (s.trill) {
        // A 16 Hz flutter with linked pitch and amplitude movement.
        const int16_t flutter = sine(uint8_t(age * 16));
        hz += flutter * 2;
        level = uint16_t(level) * (192 + flutter / 2) / 255;
      }
      hz = (hz * pitchScale + 500) / 1000;
      ramp(uint16_t(level) << 8, uint32_t(hz) << 18);
    } else {
      ramp(0, increment);
    }
    // A longer breath separates the two renditions of the motif.
    const uint16_t breath = index == count - 1 ? 90 : 0;
    if (++age >= s.ticks + s.gap + breath) {
      age = 0;
      if (++index == count * 2) active = false;
    }
  }
  int8_t audio() {
    if (remaining) {
      amplitude += amplitudeDelta;
      increment += incrementDelta;
      if (--remaining == 0) { amplitude = amplitudeTarget; increment = incrementTarget; }
    }
    phase += increment;
    return int32_t(sine(phase >> 24)) * amplitude / 65536;
  }

 private:
  struct Syllable {
    int16_t start, end, bend;
    uint8_t ticks, gap, level;
    bool trill;
  } motif[5] = {};
  uint32_t rng, phase = 0, increment = 0, incrementTarget = 0;
  uint16_t pitchScale;
  int32_t incrementDelta = 0;
  int16_t amplitude = 0, amplitudeTarget = 0, amplitudeDelta = 0;
  uint8_t remaining = 0, count = 0, index = 0;
  uint16_t age = 0;
  int16_t repeatShift = 0;
  Profile mode = Soft;
  bool active = false;
  uint16_t pick(uint16_t upper) {
    rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
    return rng % upper;
  }
  static int8_t sine(uint8_t index) {
#ifdef __AVR__
    return int8_t(pgm_read_byte(&SINE[index]));
#else
    return SINE[index];
#endif
  }
  void ramp(int16_t level, uint32_t frequencyIncrement) {
    amplitudeTarget = level;
    incrementTarget = frequencyIncrement;
    amplitudeDelta = (level - amplitude) / 64;
    incrementDelta = (int32_t(frequencyIncrement) - int32_t(increment)) / 64;
    remaining = 64;
  }
};
}  // namespace ara
