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
  uint16_t targetLevelQ8() const { return amplitudeTarget; }
  bool pitchWasLimited() const { return frequencyLimited; }
  bool longNoteActive() const { return heldActive; }
  uint8_t longNoteTicks() const { return heldSyllable == 255 ? 0 : heldTicks; }
  bool startsWithLongNote() const { return heldSyllable == 0; }
  void setNoisyOnsets(bool enabled) { noisyOnsets = enabled; }
  void setGainPercent(uint16_t percent) {
    // Uno PWM accepts almost nine bits, wider than the oscillator's table.
    if (percent > 500) percent = 500;
    gainQ8 = (uint32_t(percent) * 256 + 50) / 100;
  }
  Profile selectedProfile() const { return mode; }
  uint8_t selectedMotif() const { return mode >= Song ? melodyVariant : 255; }
  bool repeatsMelody() const { return repeatMelody; }
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
    repeatMelody = pick(5) < 3;
    beginPhrase(profile, false);
  }
 private:
  void beginPhrase(Profile profile, bool second) {
    if (variableRegister && !second) {
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
    heldSyllable = 255;
    count = profile >= Song ? 6 + pick(3) : profile == Soft ? 4 : 5;
    if (profile >= Song) {
      // Twelve contours, each available forwards/backwards and rising/falling.
      melodyVariant = pick(48);
      while (melodyVariant == recentMotifs[0] || melodyVariant == recentMotifs[1]
             || melodyVariant == recentMotifs[2]) melodyVariant = (melodyVariant + 1) % 48;
      recentMotifs[2] = recentMotifs[1];
      recentMotifs[1] = recentMotifs[0];
      recentMotifs[0] = melodyVariant;
      endingStyle = pick(4);
      phraseBreath = 20 + pick(65);
      // A held gesture in roughly three out of ten melodic phrases.
      if ((profile == Song || profile == TrillSong) && pick(10) < 3) {
        if (pick(3) == 0) {
          heldSyllable = 0;
        } else {
          heldSyllable = 1 + pick(count - 1);
        }
        heldTicks = 112 + pick(57);
        heldDrift = int16_t(pick(91)) - 45;
        heldBend = int16_t(pick(51)) - 25;
      }
    }
    const uint8_t firstTrill = profile >= Song ? pick(count) : 0;
    const uint8_t secondTrill = profile >= Song ? (firstTrill + 2 + pick(count - 2)) % count : 0;
    uint8_t groupRemaining = profile >= Song ? 1 + pick(4) : 1;
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
      s.tailStyle = 0;
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
#ifdef __AVR__
        int16_t offset = int16_t(pgm_read_word(&offsets[melodyVariant % 12][melodyVariant >= 24 ? count - 1 - i : i]));
#else
        int16_t offset = offsets[melodyVariant % 12][melodyVariant >= 24 ? count - 1 - i : i];
#endif
        if ((melodyVariant / 12) % 2) offset = -offset;
        s.start = base + (profile == Cascade ? int16_t(count - 1) * cascadeStep / 2 - int16_t(i) * cascadeStep : offset);
        s.start += int16_t(pick(81)) - 40;
        s.end = s.start + (pick(2) ? -100 - int16_t(pick(121)) : 100 + int16_t(pick(121)));
        s.bend = pick(2) ? -80 - int16_t(pick(101)) : 80 + int16_t(pick(101));
        s.level = 100 + pick(16);
        s.trill = i == firstTrill || i == secondTrill || (profile == TrillSong && pick(5) == 0);
        s.trillRate = 18 + pick(11);
        const uint8_t gesture = pick(4);
        s.ticks = s.trill ? 48 + pick(64) : gesture == 0 ? 10 + pick(10)
            : gesture == 1 ? 18 + pick(16) : gesture == 2 ? 30 + pick(24) : 45 + pick(24);
        if (profile == Cascade && !s.trill) s.ticks = 18 + pick(12);
        // Separate irregular groups of one to four gestures with breaths.
        if (--groupRemaining == 0) {
          s.gap = 18 + pick(29);
          groupRemaining = 1 + pick(4);
        } else s.gap = 2 + pick(7);
        if (i == count - 1) {
          if (endingStyle == 0) { s.ticks = 10 + pick(14); s.trill = false; s.tailStyle = 1; }
          if (endingStyle == 1) { s.ticks = 24 + pick(24); s.trill = false; s.tailStyle = 2; }
          if (endingStyle == 2) { s.ticks = 48 + pick(48); s.trill = true; }
        }
      }
    }
    index = second ? count : 0;
    age = 0;
    active = true;
  }
 public:
  void control() {
    frequencyLimited = false;
    heldActive = false;
    if (!active) { ramp(0, increment); return; }
    uint8_t syllable = index % count;
    const Syllable &s = motif[syllable];
    const bool holding = mode >= Song && syllable == heldSyllable;
    uint16_t ticks = s.ticks;
    const uint16_t gap = s.gap;
    uint8_t tailStyle = s.tailStyle;
    if (holding) { ticks = heldTicks; tailStyle = 0; }
    if (age < ticks) {
      heldActive = holding;
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
      if (holding) {
        hz = s.start + int32_t(heldDrift) * t / 255;
        hz += int32_t(heldBend) * 4 * t * (255 - t) / 65025;
      }
      uint8_t level = s.level;
      if (holding) level = uint16_t(level) * 88 / 100;
      const uint8_t attack = holding ? 6 : mode >= Song ? 2 : mode >= Chirps ? 1 : mode == Soft ? 7 : 4;
      const uint8_t release = holding ? 12 : tailStyle == 1 ? 2 : mode >= Song ? 5 : mode >= Chirps ? 4 : mode == Soft ? 12 : 7;
      if (tailStyle == 2) level = uint16_t(level) * (255 - t) / 255;
      if (mode >= Chirps && mode < Song) {
        // Short attack and a decaying body instead of a sustained whistle.
        level = uint16_t(level) * (255 - t / 2) / 255;
      }
      if (age < attack) level = uint16_t(level) * age / attack;
      if (ticks - 1 - age < release)
        level = uint16_t(level) * (ticks - 1 - age) / release;
      if (s.trill && !holding) {
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
        ? (mode >= Song ? phraseBreath : 90) : 0;
    if (++age >= ticks + gap + breath) {
      age = 0;
      ++index;
      if (index == count && !repeatMelody) {
        // Generate a fresh melody without changing the voice or register.
        beginPhrase(mode, true);
      } else if (index == count * renditions) active = false;
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
    uint8_t tailStyle;
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
  uint8_t registerWeights[3] = {44, 54, 2};
  uint8_t melodyVariant = 0;
  uint8_t recentMotifs[3] = {255, 255, 255};
  bool repeatMelody = true;
  uint8_t endingStyle = 0, phraseBreath = 55;
  uint8_t heldSyllable = 255, heldTicks = 128;
  int8_t heldDrift = 0, heldBend = 0;
  bool heldActive = false;
  int32_t incrementDelta = 0;
  int16_t amplitude = 0, amplitudeTarget = 0, amplitudeDelta = 0;
  uint8_t remaining = 0, count = 0, index = 0;
  uint16_t age = 0;
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
