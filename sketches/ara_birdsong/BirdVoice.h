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
  enum Profile : uint8_t { Soft, Garden, Lively, Chirps, Liquid, Textured };
  // 1888/1000 raises the register by approximately eleven semitones.
  explicit BirdVoice(uint32_t seed = 1, uint16_t pitchScale = 1888)
      : rng(seed ? seed : 1), pitchScale(pitchScale) {}
  bool busy() const { return active; }
  void setNoisyOnsets(bool enabled) { noisyOnsets = enabled; }
  void setGainPercent(uint16_t percent) {
    // Uno PWM accepts almost nine bits, wider than the oscillator's table.
    if (percent > 375) percent = 375;
    gainQ8 = (uint32_t(percent) * 256 + 50) / 100;
  }
  Profile selectedProfile() const { return mode; }
  void triggerRandom() {
    if (!active) trigger(static_cast<Profile>(pick(6)));
  }
  // Semitones above the original experiment. Choose once for each whole song.
  void setRegisterRange(uint8_t lower, uint8_t upper) {
    minimumSemitones = lower > 12 ? 12 : lower;
    maximumSemitones = upper > 12 ? 12 : upper;
    if (minimumSemitones > maximumSemitones) {
      const uint8_t swap = minimumSemitones;
      minimumSemitones = maximumSemitones;
      maximumSemitones = swap;
    }
    variableRegister = true;
  }
  uint8_t selectedRegister() const { return selectedSemitones; }
  void trigger(Profile profile) {
    if (active) return;
    if (variableRegister) {
      selectedSemitones = minimumSemitones + pick(maximumSemitones - minimumSemitones + 1);
      static const uint16_t scales[] ARA_FLASH = {
        1000, 1059, 1122, 1189, 1260, 1335, 1414, 1498, 1587, 1682, 1782, 1888, 2000
      };
#ifdef __AVR__
      pitchScale = pgm_read_word(&scales[selectedSemitones]);
#else
      pitchScale = scales[selectedSemitones];
#endif
    }
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
      if (profile >= Chirps) {
        // Preserve the pitch vocabulary but replace the slow whistle gestures.
        s.ticks = profile == Chirps ? 14 + pick(15) : 24 + pick(24);
        s.gap = 5 + pick(13);
        s.end = s.start - 180 - pick(400);
        s.bend = 280 + pick(350);
      }
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
      if (mode >= Chirps) {
        // Reach the high point early, then fall for the rest of the chirp.
        const uint16_t contour = t < 48 ? t * 255 / 48 : (255 - t) * 255 / 207;
        hz += int32_t(s.bend) * contour / 255;
      } else {
        hz += int32_t(s.bend) * 4 * t * (255 - t) / 65025;
      }
      if (index >= count) hz += repeatShift;
      uint8_t level = s.level;
      const uint8_t attack = mode >= Chirps ? 1 : mode == Soft ? 7 : 4;
      const uint8_t release = mode >= Chirps ? 4 : mode == Soft ? 12 : 7;
      if (mode >= Chirps) {
        // Short attack and a decaying body instead of a sustained whistle.
        level = uint16_t(level) * (255 - t / 2) / 255;
      }
      if (age < attack) level = uint16_t(level) * age / attack;
      if (s.ticks - 1 - age < release)
        level = uint16_t(level) * (s.ticks - 1 - age) / release;
      if (s.trill) {
        // A 16 Hz flutter with linked pitch and amplitude movement.
        const int16_t flutter = sine(uint8_t(age * 16));
        hz += flutter * 2;
        level = uint16_t(level) * (192 + flutter / 2) / 255;
      }
      if (mode == Liquid) {
        const int16_t flutter = sine(uint8_t(age * 39));
        hz += flutter * 2;
        level = uint16_t(level) * (208 + flutter / 3) / 255;
      }
      // A quieter second tone and a brief noisy onset are separate auditions.
      secondToneMix = mode == Liquid ? 16 : mode == Textured ? 24 : 0;
      noiseMix = noisyOnsets && mode == Textured && age < 4 ? (4 - age) * 6 : 0;
      hz = (hz * pitchScale + 500) / 1000;
      if (hz > 7000) hz = 7000;
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
  int16_t audio() {
    if (remaining) {
      amplitude += amplitudeDelta;
      increment += incrementDelta;
      if (--remaining == 0) { amplitude = amplitudeTarget; increment = incrementTarget; }
    }
    phase += increment;
    int16_t wave = sine(phase >> 24);
    if (secondToneMix) {
      secondPhase += increment - (increment >> 2);
      wave = (wave * (128 - secondToneMix) + int16_t(sine(secondPhase >> 24)) * secondToneMix) / 128;
    }
    if (noiseMix) {
      noiseState = (noiseState >> 1) ^ (uint16_t(0 - (noiseState & 1)) & 0xb400u);
      const int16_t noise = int16_t(noiseState >> 8) - 128;
      wave = (wave * (128 - noiseMix) + noise * noiseMix) / 128;
    }
    // Explicit unsigned shifts avoid AVR's slow signed division helpers.
    // Take the magnitude first to preserve truncation toward zero for negatives.
    const int32_t product = int32_t(wave) * amplitude;
    const int16_t sample = product < 0
        ? -int16_t(uint32_t(-product) >> 16)
        : int16_t(uint32_t(product) >> 16);
    const int32_t gained = int32_t(sample) * gainQ8;
    const int16_t output = gained < 0 ? -int16_t(uint32_t(-gained) >> 8)
                                     : int16_t(uint32_t(gained) >> 8);
    // Mozzi's default Uno PWM range is -244 through +243. Leave a margin.
    return output < -240 ? -240 : output > 240 ? 240 : output;
  }

 private:
  struct Syllable {
    int16_t start, end, bend;
    uint8_t ticks, gap, level;
    bool trill;
  } motif[5] = {};
  uint32_t rng, phase = 0, increment = 0, incrementTarget = 0;
  uint16_t pitchScale;
  uint16_t gainQ8 = 256;
  bool noisyOnsets = true;
  uint32_t secondPhase = 0;
  uint16_t noiseState = 0xace1;
  uint8_t secondToneMix = 0, noiseMix = 0;
  uint8_t minimumSemitones = 8, maximumSemitones = 11, selectedSemitones = 11;
  bool variableRegister = false;
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
