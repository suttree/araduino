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
  if (argc != 2) return 2;
  const std::string directory = argv[1];
  constexpr uint32_t frames = 26 * 16384;
  std::vector<int16_t> samples(frames);
  legacy::setup();
  for (sampleClock = 0; sampleClock < frames; ++sampleClock) {
    // Compress the minutes of silence for auditioning. Leave the song unchanged.
    if (sampleClock && sampleClock % (8 * 16384) == 0 && sampleClock < 24 * 16384)
      legacy::startNewSong(millis());
    if (sampleClock % 256 == 0) legacy::updateControl();
    samples[sampleClock] = legacy::updateAudio() * 256;
  }
  wav(directory + "/00-current-reference.wav", samples);
  const char *names[] = {"01-soft-whistles", "02-garden-calls", "03-lively-trills",
                         "04-soft-original-register"};
  for (int profile = 0; profile < 4; ++profile) {
    ara::BirdVoice voice(20261122, profile == 3 ? 1000 : 1888);
    int peak = 0;
    uint64_t energy = 0;
    unsigned bouts = 0;
    for (uint32_t frame = 0; frame < frames; ++frame) {
      if (frame % (8 * 16384) == 0 && frame < 24 * 16384) {
        if (voice.busy()) throw std::runtime_error("Bout overlaps next audition slot");
        voice.trigger(ara::BirdVoice::Profile(profile % 3));
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
  for (uint32_t seed = 1; seed <= 100; ++seed) {
    for (int profile = 0; profile < 3; ++profile) {
      ara::BirdVoice voice(seed);
      voice.trigger(ara::BirdVoice::Profile(profile));
      for (unsigned tick = 0; tick < 6 * 256; ++tick) {
        voice.control();
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
  std::cout << "Passed completion, restart and output-range checks for 300 phrases\n";
}
