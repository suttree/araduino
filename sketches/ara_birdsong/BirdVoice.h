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
  enum Profile : uint8_t { Soft, Garden, Lively, Chirps, Liquid, Textured,
                           Song, TrillSong, Cascade, ProfileCount };
  // 1888/1000 raises the register by approximately eleven semitones.
  explicit BirdVoice(uint32_t seed = 1, uint16_t pitchScale = 1888)
      : rng(seed ? seed : 1), pitchScale(pitchScale) {}
  bool busy() const { return active; }
  uint16_t targetFrequencyHz() const { return incrementTarget >> 18; }
  bool pitchWasLimited() const { return frequencyLimited; }
  void setNoisyOnsets(bool enabled) { noisyOnsets = enabled; }
  void setGainPercent(uint16_t percent) {
    // Uno PWM accepts almost nine bits, wider than the oscillator's table.
    if (percent > 500) percent = 500;
    gainQ8 = (uint32_t(percent) * 256 + 50) / 100;
  }
  Profile selectedProfile() const { return mode; }
  uint8_t selectedMotif() const { return mode >= Song ? melodyVariant : 255; }
  uint8_t selectedRenditions() const { return 2; }
  uint8_t currentRendition() const { return count ? index / count + 1 : 0; }
  void triggerRandom() {
    if (active) return;
    // Repeat prevention applies to tunes, so melodic voices can follow each other.
    trigger(randomProfile());
  }
  // Semitones above the original experiment. Choose once for each whole song.
  void setRegisterRange(uint8_t lower, uint8_t upper) {
    minimumSemitones = lower > 17 ? 17 : lower;
    maximumSemitones = upper > 17 ? 17 : upper;
    if (minimumSemitones > maximumSemitones) {
      const uint8_t swap = minimumSemitones;
      minimumSemitones = maximumSemitones;
      maximumSemitones = swap;
    }
    variableRegister = true;
    weightedRegisters = false;
  }
  void setRegisterChoices(uint8_t lower, uint8_t middle, uint8_t upper,
                          uint8_t lowerWeight, uint8_t middleWeight, uint8_t upperWeight) {
    registerChoices[0] = lower > 17 ? 17 : lower;
    registerChoices[1] = middle > 17 ? 17 : middle;
    registerChoices[2] = upper > 17 ? 17 : upper;
    registerWeights[0] = lowerWeight;
    registerWeights[1] = middleWeight;
    registerWeights[2] = upperWeight;
    if (uint16_t(lowerWeight) + middleWeight + upperWeight == 0)
      registerWeights[0] = registerWeights[1] = registerWeights[2] = 1;
    variableRegister = weightedRegisters = true;
  }
  uint8_t selectedRegister() const { return selectedSemitones; }
  void trigger(Profile profile) {
    if (active) return;
    if (variableRegister) {
      if (weightedRegisters) {
        const uint16_t total = uint16_t(registerWeights[0]) + registerWeights[1] + registerWeights[2];
        const uint16_t choice = pick(total);
        selectedSemitones = choice < registerWeights[0] ? registerChoices[0]
            : choice < uint16_t(registerWeights[0]) + registerWeights[1] ? registerChoices[1]
                                                                                  : registerChoices[2];
      } else {
        selectedSemitones = minimumSemitones + pick(maximumSemitones - minimumSemitones + 1);
      }
      static const uint16_t scales[] ARA_FLASH = {
        1000, 1059, 1122, 1189, 1260, 1335, 1414, 1498, 1587, 1682, 1782, 1888, 2000,
        2119, 2245, 2378, 2520, 2670
      };
#ifdef __AVR__
      pitchScale = pgm_read_word(&scales[selectedSemitones]);
#else
      pitchScale = scales[selectedSemitones];
#endif
    }
    mode = profile;
    count = profile >= Song ? (profile == Cascade ? 6 + pick(3) : 7 + pick(2)) : profile == Soft ? 4 : 5;
    if (profile >= Song) {
      // Twelve contours, each available forwards/backwards and rising/falling.
      melodyVariant = pick(48);
      while (melodyVariant == recentMotifs[0] || melodyVariant == recentMotifs[1]
             || melodyVariant == recentMotifs[2]) melodyVariant = (melodyVariant + 1) % 48;
      recentMotifs[2] = recentMotifs[1];
      recentMotifs[1] = recentMotifs[0];
      recentMotifs[0] = melodyVariant;
      rhythmPattern = pick(4);
      phraseOrder = pick(4);
      answerTempo = 90 + pick(21);
      answerTranspose = int16_t(pick(241)) - 120;
    }
    const uint8_t firstTrill = profile >= Song ? 2 + pick(2) : 0;
    const uint8_t cascadeStep = profile >= Song ? 70 + pick(61) : 100;
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
      s.trillRate = 16;
      if (s.trill) s.ticks += 28;
      if (profile >= Chirps) {
        // Preserve the pitch vocabulary but replace the slow whistle gestures.
        s.ticks = profile == Chirps ? 14 + pick(15) : 24 + pick(24);
        s.gap = 5 + pick(13);
        s.end = s.start - 180 - pick(400);
        s.bend = 280 + pick(350);
      }
      if (profile >= Song) {
        // Choose a whole contour, then vary the rhythm and syllables within it.
        static const int16_t offsets[][8] ARA_FLASH = {
          {0, 180, 300, 140, -120, 80, 240, -160},
          {220, 60, -180, 0, 260, 100, -80, 180},
          {-180, 0, 220, -100, 80, 280, 140, -60},
          {100, 280, 40, -200, -40, 180, -120, 60},
          {280, 100, -60, 200, 0, -180, 60, 240},
          {-80, 180, 0, 260, 80, -140, 200, 40},
          {160, -40, 240, 80, -180, 140, 0, 280},
          {-140, 100, 260, 40, 180, -80, 300, 60},
          {240, 0, -160, 120, 280, 60, -60, 160},
          {40, 260, 120, -100, 200, 0, 300, 80},
          {-200, 60, 180, 300, 100, -40, 220, 0},
          {180, 300, 60, -120, 40, 240, -80, 100}
        };
        static const uint8_t rhythms[][8] ARA_FLASH = {
          {0, 8, 0, 12, 4, 0, 10, 6}, {10, 0, 6, 0, 12, 4, 0, 8},
          {4, 12, 0, 8, 0, 10, 6, 0}, {8, 4, 10, 0, 6, 0, 12, 4}
        };
#ifdef __AVR__
        int16_t offset = int16_t(pgm_read_word(&offsets[melodyVariant % 12][melodyVariant >= 24 ? count - 1 - i : i]));
        const uint8_t rhythm = pgm_read_byte(&rhythms[rhythmPattern][i]);
#else
        int16_t offset = offsets[melodyVariant % 12][melodyVariant >= 24 ? count - 1 - i : i];
        const uint8_t rhythm = rhythms[rhythmPattern][i];
#endif
        if ((melodyVariant / 12) % 2) offset = -offset;
        s.start = base + (profile == Cascade ? int16_t(count - 1) * cascadeStep / 2 - int16_t(i) * cascadeStep : offset);
        s.start += int16_t(pick(81)) - 40;
        s.end = s.start + (pick(2) ? -100 - int16_t(pick(121)) : 100 + int16_t(pick(121)));
        s.bend = pick(2) ? -80 - int16_t(pick(101)) : 80 + int16_t(pick(101));
        s.level = 100 + pick(16);
        s.trill = i == firstTrill || i == count - 2 || (profile == TrillSong && (i == 1 || i == count / 2));
        s.trillRate = 18 + pick(11);
        s.ticks = s.trill ? 65 + pick(40) + rhythm : 18 + pick(24) + rhythm;
        if (i == 0 || i == count - 1) s.ticks = 30 + pick(24);
        if (profile == Cascade && !s.trill) s.ticks = 18 + pick(12);
        s.gap = 4 + pick(7);
        if (i == count / 2) s.gap += 12 + pick(17);
      }
    }
    index = 0;
    age = 0;
    repeatShift = pick(61) - 30;
    active = true;
  }
  void control() {
    frequencyLimited = false;
    if (!active) { ramp(0, increment); return; }
    uint8_t syllable = index % count;
    if (mode >= Song && index >= count) {
      if (phraseOrder == 1) syllable = (syllable + count / 2) % count;
      if (phraseOrder == 2 && syllable >= count - 2)
        syllable = count * 2 - 3 - syllable;
      if (phraseOrder == 3 && (syllable % 2 || syllable + 1 < count)) syllable ^= 1;
    }
    const Syllable &s = motif[syllable];
    const bool answering = mode >= Song && index >= count;
    const uint16_t ticks = answering ? (uint16_t(s.ticks) * answerTempo + 50) / 100 : s.ticks;
    const uint16_t gap = answering ? (uint16_t(s.gap) * answerTempo + 50) / 100 : s.gap;
    if (age < ticks) {
      const uint16_t t = uint32_t(age) * 255 / (ticks - 1);
      // Keep the fraction until after multiplication by the bend depth.
      int32_t hz = s.start + int32_t(s.end - s.start) * t / 255;
      if (mode >= Chirps && mode < Song) {
        // Reach the high point early, then fall for the rest of the chirp.
        const uint16_t contour = t < 48 ? t * 255 / 48 : (255 - t) * 255 / 207;
        hz += int32_t(s.bend) * contour / 255;
      } else {
        hz += int32_t(s.bend) * 4 * t * (255 - t) / 65025;
      }
      if (index >= count) hz += repeatShift;
      if (answering) hz += answerTranspose;
      uint8_t level = s.level;
      const uint8_t attack = mode >= Song ? 2 : mode >= Chirps ? 1 : mode == Soft ? 7 : 4;
      const uint8_t release = mode >= Song ? 5 : mode >= Chirps ? 4 : mode == Soft ? 12 : 7;
      if (mode >= Chirps && mode < Song) {
        // Short attack and a decaying body instead of a sustained whistle.
        level = uint16_t(level) * (255 - t / 2) / 255;
      }
      if (age < attack) level = uint16_t(level) * age / attack;
      if (ticks - 1 - age < release)
        level = uint16_t(level) * (ticks - 1 - age) / release;
      if (s.trill) {
        const int16_t flutter = sine(uint8_t(age * s.trillRate));
        hz += mode >= Song ? flutter : flutter * 2;
        // Song trills fall almost to silence between pulses, unlike vibrato.
        level = uint16_t(level) * (mode >= Song ? 128 + flutter : 192 + flutter / 2) / 255;
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
      if (hz > 7000) { hz = 7000; frequencyLimited = true; }
      ramp(uint16_t(level) << 8, uint32_t(hz) << 18);
    } else {
      ramp(0, increment);
    }
    // A longer breath separates the two renditions of the motif.
    const uint8_t renditions = selectedRenditions();
    const uint16_t breath = (index + 1) % count == 0 && index + 1 < count * renditions
        ? (mode >= Song ? 55 : 90) : 0;
    if (++age >= ticks + gap + breath) {
      age = 0;
      if (++index == count * renditions) active = false;
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
    uint8_t trillRate;
  } motif[8] = {};
  uint32_t rng, phase = 0, increment = 0, incrementTarget = 0;
  uint16_t pitchScale;
  uint16_t gainQ8 = 256;
  bool noisyOnsets = true;
  uint32_t secondPhase = 0;
  uint16_t noiseState = 0xace1;
  uint8_t secondToneMix = 0, noiseMix = 0;
  uint8_t minimumSemitones = 8, maximumSemitones = 11, selectedSemitones = 11;
  bool variableRegister = false;
  bool weightedRegisters = false, frequencyLimited = false;
  uint8_t registerChoices[3] = {13, 15, 17};
  uint8_t registerWeights[3] = {4, 5, 1};
  uint8_t melodyVariant = 0, rhythmPattern = 0;
  uint8_t recentMotifs[3] = {255, 255, 255};
  uint8_t phraseOrder = 0, answerTempo = 100;
  int16_t answerTranspose = 0;
  int32_t incrementDelta = 0;
  int16_t amplitude = 0, amplitudeTarget = 0, amplitudeDelta = 0;
  uint8_t remaining = 0, count = 0, index = 0;
  uint16_t age = 0;
  int16_t repeatShift = 0;
  Profile mode = Soft;
  bool active = false;
  Profile randomProfile() {
    const uint8_t choice = pick(100);
    return choice < 45 ? Song : choice < 90 ? TrillSong : choice < 97 ? Cascade
                                                               : static_cast<Profile>(pick(6));
  }
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
