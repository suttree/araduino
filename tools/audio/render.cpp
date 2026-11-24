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
void renderSongPreviews(const std::string &directory) {
  const ara::BirdVoice::Profile profiles[] = {ara::BirdVoice::Soft, ara::BirdVoice::Song,
      ara::BirdVoice::TrillSong, ara::BirdVoice::Cascade};
  const char *names[] = {"00-previous-whistle", "01-motif-song", "02-trill-song", "03-cascade-song"};
  for (unsigned variant = 0; variant < 5; ++variant) {
    ara::BirdVoice voice(20261123);
    voice.setGainPercent(400);
    voice.setNoisyOnsets(false);
    if (variant == 4) voice.setRegisterRange(8, 11);
    const unsigned seconds = variant == 4 ? 40 : 12;
    std::vector<int16_t> samples(seconds * 16384);
    for (unsigned frame = 0; frame < samples.size(); ++frame) {
      if (frame == 0 || (variant == 4 && frame % (13 * 16384) == 0 && frame < 39 * 16384)) {
        if (voice.busy()) throw std::runtime_error("Song overlaps the next audition slot");
        if (variant == 4) {
          voice.triggerRandom();
          std::cout << "Mixed song at " << frame / 16384 << "s: profile "
                    << int(voice.selectedProfile()) << ", register +" << int(voice.selectedRegister()) << "\n";
        } else voice.trigger(profiles[variant]);
      }
      if (frame % 64 == 0) voice.control();
      const int sample = voice.audio();
      if (std::abs(sample) > 240) throw std::runtime_error("Song exceeds PWM range");
      // Map the almost-nine-bit PWM range to PCM without wrapping above 127.
      samples[frame] = sample * 128;
    }
    if (voice.busy() || voice.audio() != 0) throw std::runtime_error("Song has not finished");
    wav(directory + "/" + (variant == 4 ? "04-random-song-mix" : names[variant]) + ".wav", samples);
  }
}

void renderRegisterPreviews(const std::string &directory) {
  // Fixed offsets above the original experiment, compared with today's upper +11 register.
  const uint16_t scales[] = {1888, 2119, 2378, 2670};
  const char *names[] = {"00-current-plus11", "01-higher-plus13", "02-higher-plus15", "03-higher-plus17"};
  const ara::BirdVoice::Profile profiles[] = {ara::BirdVoice::Song, ara::BirdVoice::TrillSong,
                                            ara::BirdVoice::Cascade};
  std::ofstream report(directory + "/register-peaks.csv");
  report << "file,scale,peak_hz,ceiling_updates\n";
  for (unsigned variant = 0; variant < 4; ++variant) {
    std::vector<int16_t> samples(33 * 16384);
    unsigned peakHz = 0, ceilingUpdates = 0;
    for (unsigned song = 0; song < 3; ++song) {
      ara::BirdVoice voice(20261123, scales[variant]);
      voice.setGainPercent(400);
      voice.setNoisyOnsets(false);
      voice.trigger(profiles[song]);
      for (unsigned frame = 0; frame < 11 * 16384; ++frame) {
        if (frame % 64 == 0) {
          voice.control();
          if (voice.busy()) {
            peakHz = std::max(peakHz, unsigned(voice.targetFrequencyHz()));
            if (voice.targetFrequencyHz() == 7000) ++ceilingUpdates;
          }
        }
        const int sample = voice.audio();
        if (std::abs(sample) > 240) throw std::runtime_error("Register preview exceeds PWM range");
        samples[song * 11 * 16384 + frame] = sample * 128;
      }
      if (voice.busy() || voice.audio() != 0)
        throw std::runtime_error("Register preview cuts off a song");
    }
    if (ceilingUpdates) throw std::runtime_error("Higher-register sample hits the pitch ceiling");
    wav(directory + "/" + names[variant] + ".wav", samples);
    report << names[variant] << ',' << scales[variant] << ',' << peakHz << ',' << ceilingUpdates << '\n';
    std::cout << names[variant] << ": peak pitch " << peakHz << " Hz; "
              << ceilingUpdates << " control updates at pitch ceiling\n";
  }
  if (!report) throw std::runtime_error("Could not write register pitch report");
}

void renderMelodySession(const std::string &directory, uint32_t seed) {
  ara::BirdVoice voice(seed);
  voice.setGainPercent(350);
  voice.setNoisyOnsets(false);
  voice.setRegisterChoices(13, 15, 17, 4, 5, 1);
  std::vector<int16_t> samples;
  unsigned frame = 0;
  auto step = [&]() {
    if (frame % 64 == 0) voice.control();
    const int sample = voice.audio();
    if (std::abs(sample) > 240) throw std::runtime_error("Session exceeds output range");
    samples.push_back(sample * 128);
    ++frame;
  };
  std::ofstream report(directory + "/melody-session.csv");
  report << "song,start_seconds,duration_seconds,profile,motif,register,max_rendition,seed\n";
  // Showcase the melodic repertoire; this order is illustrative, not a random-frequency sample.
  const ara::BirdVoice::Profile examples[] = {ara::BirdVoice::Song, ara::BirdVoice::TrillSong,
      ara::BirdVoice::Song, ara::BirdVoice::TrillSong, ara::BirdVoice::Song,
      ara::BirdVoice::Cascade, ara::BirdVoice::TrillSong, ara::BirdVoice::Song,
      ara::BirdVoice::TrillSong, ara::BirdVoice::Song};
  for (unsigned song = 0; song < 10; ++song) {
    const unsigned start = frame;
    voice.trigger(examples[song]);
    const auto profile = voice.selectedProfile();
    const unsigned motif = voice.selectedMotif(), pitch = voice.selectedRegister();
    unsigned maximumRendition = 0;
    do {
      if (voice.busy()) maximumRendition = std::max(maximumRendition, unsigned(voice.currentRendition()));
      step();
      if (frame - start > 12 * 16384) throw std::runtime_error("Demo song did not finish");
    } while (voice.busy());
    if (maximumRendition > 2) throw std::runtime_error("Demo repeated a melody more than twice");
    report << song + 1 << ',' << double(start) / 16384 << ',' << double(frame - start) / 16384
           << ',' << int(profile) << ',' << motif << ',' << pitch << ',' << maximumRendition
           << ',' << seed << '\n';
    if (song < 9) for (unsigned gap = 0; gap < 2 * 16384; ++gap) step();
  }
  for (unsigned tail = 0; tail < 16384; ++tail) step();
  if (voice.audio() != 0 || !report) throw std::runtime_error("Demo did not finish cleanly");
  wav(directory + "/ara-melody-session.wav", samples);
  std::cout << "Rendered ten melody and trill examples, " << double(frame) / 16384 << " seconds\n";
}

int main(int argc, char **argv) {
  if (argc < 2 || argc > 4) return 2;
  const std::string directory = argv[1];
  const bool variations = argc == 3 && std::string(argv[2]) == "--variations";
  const bool songs = argc == 3 && std::string(argv[2]) == "--songs";
  const bool registers = argc == 3 && std::string(argv[2]) == "--registers";
  const bool session = argc >= 3 && std::string(argv[2]) == "--session";
  if (argc == 4 && !session) return 2;
  if (session) renderMelodySession(directory, argc == 4 ? std::stoul(argv[3]) : 20261124);
  else if (registers) renderRegisterPreviews(directory);
  else if (songs) renderSongPreviews(directory);
  else {
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
  }
  // Exercise phrase completion and silence over more than the audition seeds.
  unsigned registersSeen = 0;
  for (uint32_t seed = 1; seed <= 100; ++seed) {
    for (int profile = 0; profile < ara::BirdVoice::ProfileCount; ++profile) {
      for (unsigned variable = 0; variable < 2; ++variable) {
        ara::BirdVoice voice(seed);
        if (variable) voice.setRegisterRange(8, 11);
        voice.trigger(ara::BirdVoice::Profile(profile));
        const uint8_t selected = voice.selectedRegister();
        if (variable) {
          if (selected < 8 || selected > 11) throw std::runtime_error("Register out of range");
          registersSeen |= 1u << (selected - 8);
        }
        const unsigned limit = profile >= ara::BirdVoice::Song ? 12 : 6;
        for (unsigned tick = 0; tick < limit * 256; ++tick) {
          voice.control();
          if (voice.busy() && voice.currentRendition() > 2)
            throw std::runtime_error("A melody repeated more than twice");
          if (voice.selectedRegister() != selected)
            throw std::runtime_error("Register changed during a song");
          for (unsigned frame = 0; frame < 64; ++frame) {
            if (std::abs(int(voice.audio())) > 64)
              throw std::runtime_error("Output exceeded expected headroom");
          }
        }
        if (voice.busy() || voice.audio() != 0)
          throw std::runtime_error("Phrase exceeded its duration limit or left nonzero output");
        voice.trigger(ara::BirdVoice::Profile(profile));
        if (!voice.busy()) throw std::runtime_error("Voice could not restart");
      }
    }
  }
  if (registersSeen != 15) throw std::runtime_error("Did not exercise all four registers");
  std::cout << "Passed completion, restart, register and output-range checks for 1800 phrases\n";
  ara::BirdVoice mixedVoice(20261122);
  mixedVoice.setGainPercent(350);
  mixedVoice.setNoisyOnsets(false);
  mixedVoice.setRegisterChoices(13, 15, 17, 4, 5, 1);
  unsigned profilesSeen = 0;
  uint8_t recentMotifs[3] = {255, 255, 255};
  uint64_t motifsSeen = 0;
  unsigned registerCounts[3] = {}, limitedTicks = 0, activeTicks = 0;
  int mixedPeak = 0;
  unsigned longerSongs = 0, melodicSongs = 0;
  for (unsigned song = 0; song < 200; ++song) {
    mixedVoice.triggerRandom();
    const auto profile = mixedVoice.selectedProfile();
    const uint8_t motif = mixedVoice.selectedMotif();
    if (profile >= ara::BirdVoice::Song) {
      if (motif >= 48 || motif == recentMotifs[0] || motif == recentMotifs[1] || motif == recentMotifs[2])
        throw std::runtime_error("Song contour repeated or was outside the repertoire");
      recentMotifs[2] = recentMotifs[1]; recentMotifs[1] = recentMotifs[0]; recentMotifs[0] = motif;
      motifsSeen |= uint64_t(1) << motif;
    }
    const uint8_t selected = mixedVoice.selectedRegister();
    const int registerIndex = selected == 13 ? 0 : selected == 15 ? 1 : selected == 17 ? 2 : -1;
    if (registerIndex < 0) throw std::runtime_error("Weighted choice selected an unexpected register");
    ++registerCounts[registerIndex];
    if (profile >= ara::BirdVoice::ProfileCount) throw std::runtime_error("Invalid random profile");
    if (profile >= ara::BirdVoice::Song) ++longerSongs;
    if (profile == ara::BirdVoice::Song || profile == ara::BirdVoice::TrillSong) ++melodicSongs;
    profilesSeen |= 1u << profile;
    // A second request during a song must leave its voice and register alone.
    mixedVoice.triggerRandom();
    for (unsigned tick = 0; tick < 12 * 256; ++tick) {
      mixedVoice.control();
      if (mixedVoice.busy() && mixedVoice.currentRendition() > 2)
        throw std::runtime_error("Random melody repeated more than twice");
      if (mixedVoice.busy()) ++activeTicks;
      if (mixedVoice.pitchWasLimited()) ++limitedTicks;
      if (mixedVoice.targetFrequencyHz() > 7000) throw std::runtime_error("Pitch exceeds frequency limit");
      if (mixedVoice.selectedProfile() != profile || mixedVoice.selectedRegister() != selected
          || mixedVoice.selectedMotif() != motif)
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
  const unsigned songProfiles = (1u << ara::BirdVoice::Song) | (1u << ara::BirdVoice::TrillSong) | (1u << ara::BirdVoice::Cascade);
  if ((profilesSeen & songProfiles) != songProfiles)
    throw std::runtime_error("Random selection did not exercise each longer-song profile");
  unsigned motifCount = 0;
  for (unsigned i = 0; i < 48; ++i) if (motifsSeen & (uint64_t(1) << i)) ++motifCount;
  if (motifCount < 36) throw std::runtime_error("Random songs used too little of the contour repertoire");
  if (longerSongs < 180 || melodicSongs < 160 || 200 - longerSongs > 15)
    throw std::runtime_error("Random selection did not favour melodies and trills");
  if (mixedPeak <= 127) throw std::runtime_error("High gain never used the wider PWM range");
  std::cout << "Passed 200 random songs at 350% gain: " << melodicSongs << " melody/trill, "
            << 200 - longerSongs << " shorter calls, " << motifCount << " contours, peak " << mixedPeak << "/244\n";
  std::cout << "Weighted registers: +13=" << registerCounts[0] << ", +15=" << registerCounts[1]
            << ", +17=" << registerCounts[2] << "; pitch limit used on " << limitedTicks
            << "/" << activeTicks << " active control ticks\n";
  unsigned choices[3] = {};
  for (unsigned seed = 1; seed <= 5000; ++seed) {
    ara::BirdVoice voice(seed);
    voice.setRegisterChoices(13, 15, 17, 4, 5, 1);
    voice.trigger(ara::BirdVoice::Song);
    const unsigned selected = voice.selectedRegister();
    if (selected == 13) ++choices[0];
    else if (selected == 15) ++choices[1];
    else if (selected == 17) ++choices[2];
    else throw std::runtime_error("Register choice is outside the requested set");
  }
  if (choices[0] < 1900 || choices[0] > 2100 || choices[1] < 2400 || choices[1] > 2600
      || choices[2] < 400 || choices[2] > 600)
    throw std::runtime_error("Weighted register distribution is outside its expected range");
  std::cout << "5000 register choices: " << choices[0] << ", " << choices[1] << ", " << choices[2] << "\n";
  // Requests beyond the gain limit must clamp, rather than wrap at 8 bits.
  for (int profile = 0; profile < ara::BirdVoice::ProfileCount; ++profile) {
    ara::BirdVoice maximum(20261122), excessive(20261122);
    maximum.setGainPercent(500);
    excessive.setGainPercent(65535);
    maximum.trigger(static_cast<ara::BirdVoice::Profile>(profile));
    excessive.trigger(static_cast<ara::BirdVoice::Profile>(profile));
    for (unsigned frame = 0; frame < 12 * 16384; ++frame) {
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
