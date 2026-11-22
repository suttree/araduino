#include <MozziGuts.h>
#include "BirdVoice.h"

static_assert(AUDIO_RATE == 16384, "This voice requires 16384 Hz audio.");
constexpr uint16_t CONTROL_RATE_HZ = 256;
// 100 is the original preview level. 200 is the maximum supported gain.
constexpr uint8_t OUTPUT_GAIN_PERCENT = 200;
// Disable the experimental noisy onset while testing physical playback.
constexpr bool NOISY_ONSETS = false;
// Each song chooses an integer semitone offset within this inclusive range.
constexpr uint8_t MIN_REGISTER_SEMITONES = 8;
constexpr uint8_t MAX_REGISTER_SEMITONES = 11;
ara::BirdVoice voice;
unsigned long lastAttempt = 0;
unsigned long attemptInterval = 0;

void scheduleAttempt(unsigned long now) {
  lastAttempt = now;
  attemptInterval = random(170000L, 370001L);
}

void updateControl() {
  const unsigned long now = millis();
  if (!voice.busy() && now - lastAttempt >= attemptInterval) {
    scheduleAttempt(now);
    if (random(2) == 0) voice.triggerRandom();
  }
  voice.control();
}

int updateAudio() { return voice.audio(); }

void setup() {
  const unsigned long seed = analogRead(0);
  randomSeed(seed);
  voice = ara::BirdVoice(seed);
  voice.setGainPercent(OUTPUT_GAIN_PERCENT);
  voice.setNoisyOnsets(NOISY_ONSETS);
  voice.setRegisterRange(MIN_REGISTER_SEMITONES, MAX_REGISTER_SEMITONES);
  voice.triggerRandom();
  scheduleAttempt(millis());
  startMozzi(CONTROL_RATE_HZ);
}

void loop() { audioHook(); }
