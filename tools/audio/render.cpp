#include <cmath>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include "../../sketches/ara_birdsong/BirdVoice.h"

// Desktop adapters execute the unchanged original sketch for the reference.
static uint32_t sampleClock = 0, legacyRng = 1;
unsigned long millis() { return uint64_t(sampleClock) * 1000 / 16384; }
int analogRead(int) { return 1; }
void randomSeed(unsigned long seed) { legacyRng = seed ? seed : 1; }
long random(long upper) {
  legacyRng ^= legacyRng << 13; legacyRng ^= legacyRng >> 17; legacyRng ^= legacyRng << 5;
  return legacyRng % upper;
}
long random(long lower, long upper) { return lower + random(upper - lower); }
void startMozzi(int) {}
void audioHook() {}
template<int N, int Rate> class Oscil {
  const int8_t *table;
  uint32_t phase = 0, step = 0;
 public:
  explicit Oscil(const int8_t *data) : table(data) {}
  void setFreq(int hz) { step = uint32_t(hz) * ((65536UL * N) / Rate); }
  int8_t next() { phase += step; return table[(phase >> 16) & (N - 1)]; }
};
namespace legacy {
void advancePlayback(unsigned long);
void startNewSong(unsigned long);
#include "../../sketches/araduino/araduino.ino"
}

void little(std::ofstream &out, uint32_t value, unsigned bytes) {
  while (bytes--) { out.put(char(value & 255)); value >>= 8; }
}
void wav(const std::string &path, const std::vector<int16_t> &data) {
  std::ofstream out(path, std::ios::binary);
  out.write("RIFF", 4); little(out, 36 + data.size() * 2, 4);
  out.write("WAVEfmt ", 8); little(out, 16, 4); little(out, 1, 2); little(out, 1, 2);
  little(out, 16384, 4); little(out, 32768, 4); little(out, 2, 2); little(out, 16, 2);
  out.write("data", 4); little(out, data.size() * 2, 4);
  for (int16_t value : data) little(out, uint16_t(value), 2);
  if (!out) throw std::runtime_error("Could not write " + path);
}
int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) return 2;
  const std::string directory = argv[1];
  const bool variations = argc == 3 && std::string(argv[2]) == "--variations";
  constexpr uint32_t frames = 26 * 16384;
  std::vector<int16_t> samples(frames);
  if (!variations) {
    legacy::setup();
    for (sampleClock = 0; sampleClock < frames; ++sampleClock) {
      // Compress the minutes of silence for auditioning. Leave the song unchanged.
      if (sampleClock && sampleClock % (8 * 16384) == 0 && sampleClock < 24 * 16384)
        legacy::startNewSong(millis());
      if (sampleClock % 256 == 0) legacy::updateControl();
      samples[sampleClock] = legacy::updateAudio() * 256;
    }
    wav(directory + "/00-current-reference.wav", samples);
  }
  const char *originalNames[] = {"01-soft-whistles", "02-garden-calls", "03-lively-trills",
                         "04-soft-original-register"};
  const char *shapeNames[] = {"00-whistle-reference", "01-short-chirps", "02-liquid-calls",
                              "03-textured-chirps", "04-variable-register"};
  const char **names = variations ? shapeNames : originalNames;
  const ara::BirdVoice::Profile shapes[] = {ara::BirdVoice::Soft, ara::BirdVoice::Chirps,
      ara::BirdVoice::Liquid, ara::BirdVoice::Textured, ara::BirdVoice::Chirps};
  for (int profile = 0; profile < (variations ? 5 : 4); ++profile) {
    ara::BirdVoice voice(20261122, !variations && profile == 3 ? 1000 : 1888);
    if (variations && profile == 4) voice.setRegisterRange(8, 11);
    int peak = 0;
    uint64_t energy = 0;
    unsigned bouts = 0;
    for (uint32_t frame = 0; frame < frames; ++frame) {
      if (frame % (8 * 16384) == 0 && frame < 24 * 16384) {
        if (voice.busy()) throw std::runtime_error("Bout overlaps next audition slot");
        voice.trigger(variations ? shapes[profile] : ara::BirdVoice::Profile(profile % 3));
        if (variations && profile == 4)
          std::cout << "Variable song " << bouts + 1 << ": +" << int(voice.selectedRegister()) << " semitones\n";
        ++bouts;
      }
      if (frame % 64 == 0) voice.control();
      const int value = voice.audio();
      peak = std::max(peak, std::abs(value));
      energy += value * value;
      samples[frame] = value * 256;
      if (frame > 25 * 16384 && value != 0) throw std::runtime_error("Voice did not settle to silence");
    }
    if (peak == 0 || peak > 64 || energy == 0 || bouts != 3)
      throw std::runtime_error("Unexpected output level");
    wav(directory + "/" + names[profile] + ".wav", samples);
    std::cout << names[profile] << ": peak " << peak << "/128, three bouts, silent tail\n";
  }
  // Exercise phrase completion and silence over more than the audition seeds.
  unsigned registersSeen = 0;
  for (uint32_t seed = 1; seed <= 100; ++seed) {
    for (int profile = 0; profile < 6; ++profile) {
      for (unsigned variable = 0; variable < 2; ++variable) {
        ara::BirdVoice voice(seed);
        if (variable) voice.setRegisterRange(8, 11);
        voice.trigger(ara::BirdVoice::Profile(profile));
        const uint8_t selected = voice.selectedRegister();
        if (variable) {
          if (selected < 8 || selected > 11) throw std::runtime_error("Register out of range");
          registersSeen |= 1u << (selected - 8);
        }
        for (unsigned tick = 0; tick < 6 * 256; ++tick) {
          voice.control();
          if (voice.selectedRegister() != selected)
            throw std::runtime_error("Register changed during a song");
          for (unsigned frame = 0; frame < 64; ++frame) {
            if (std::abs(int(voice.audio())) > 64)
              throw std::runtime_error("Output exceeded expected headroom");
          }
        }
        if (voice.busy() || voice.audio() != 0)
          throw std::runtime_error("Phrase exceeded six seconds or left nonzero output");
        voice.trigger(ara::BirdVoice::Profile(profile));
        if (!voice.busy()) throw std::runtime_error("Voice could not restart");
      }
    }
  }
  if (registersSeen != 15) throw std::runtime_error("Did not exercise all four registers");
  std::cout << "Passed completion, restart, register and output-range checks for 1200 phrases\n";
  ara::BirdVoice mixedVoice(20261122);
  mixedVoice.setGainPercent(500);
  mixedVoice.setNoisyOnsets(false);
  mixedVoice.setRegisterRange(8, 11);
  unsigned profilesSeen = 0;
  int mixedPeak = 0;
  for (unsigned song = 0; song < 100; ++song) {
    mixedVoice.triggerRandom();
    const auto profile = mixedVoice.selectedProfile();
    const uint8_t selected = mixedVoice.selectedRegister();
    if (profile > ara::BirdVoice::Textured) throw std::runtime_error("Invalid random profile");
    profilesSeen |= 1u << profile;
    // A second request during a song must leave its voice and register alone.
    mixedVoice.triggerRandom();
    for (unsigned tick = 0; tick < 6 * 256; ++tick) {
      mixedVoice.control();
      if (mixedVoice.selectedProfile() != profile || mixedVoice.selectedRegister() != selected)
        throw std::runtime_error("Voice or register changed within a song");
      for (unsigned frame = 0; frame < 64; ++frame) {
        const int output = std::abs(int(mixedVoice.audio()));
        mixedPeak = std::max(mixedPeak, output);
        if (output > 240)
          throw std::runtime_error("Mixed voice exceeded output headroom");
      }
    }
    if (mixedVoice.busy() || mixedVoice.audio() != 0)
      throw std::runtime_error("Mixed voice did not finish cleanly");
  }
  if (profilesSeen != 63) throw std::runtime_error("Random selection did not exercise every profile");
  if (mixedPeak <= 127) throw std::runtime_error("High gain never used the wider PWM range");
  if (mixedPeak != 240) throw std::runtime_error("500% gain did not exercise output limiting");
  std::cout << "Passed 100 consecutive random songs at 500% gain, limited peak " << mixedPeak
            << "/244, without noisy onsets, including every voice and trills\n";
  // Requests beyond the gain limit must clamp, rather than wrap at 8 bits.
  for (int profile = 0; profile < 6; ++profile) {
    ara::BirdVoice maximum(20261122), excessive(20261122);
    maximum.setGainPercent(500);
    excessive.setGainPercent(65535);
    maximum.trigger(static_cast<ara::BirdVoice::Profile>(profile));
    excessive.trigger(static_cast<ara::BirdVoice::Profile>(profile));
    for (unsigned frame = 0; frame < 6 * 16384; ++frame) {
      if (frame % 64 == 0) { maximum.control(); excessive.control(); }
      const int sample = maximum.audio();
      if (sample != excessive.audio() || std::abs(sample) > 240)
        throw std::runtime_error("Gain limit wrapped or exceeded the Uno PWM range");
    }
  }
  // Turning off experimental noise must affect Textured, but not the pure voices.
  for (const auto profile : {ara::BirdVoice::Soft, ara::BirdVoice::Textured}) {
    ara::BirdVoice original(20261122), clean(20261122);
    clean.setNoisyOnsets(false);
    original.trigger(profile);
    clean.trigger(profile);
    bool different = false;
    for (unsigned frame = 0; frame < 6 * 16384; ++frame) {
      if (frame % 64 == 0) { original.control(); clean.control(); }
      if (original.audio() != clean.audio()) different = true;
    }
    if (different != (profile == ara::BirdVoice::Textured))
      throw std::runtime_error("Noisy onset switch affected the wrong voice");
  }
}
