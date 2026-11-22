#include <MozziGuts.h>
#include "BirdVoice.h"

static_assert(AUDIO_RATE == 16384, "This voice requires 16384 Hz audio.");
constexpr uint16_t CONTROL_RATE_HZ = 256;
// Each song chooses an integer semitone offset within this inclusive range.
constexpr uint8_t MIN_REGISTER_SEMITONES = 8;
constexpr uint8_t MAX_REGISTER_SEMITONES = 11;
// Keep the previous voice while auditioning Chirps, Liquid and Textured.
constexpr ara::BirdVoice::Profile VOICE_PROFILE = ara::BirdVoice::Soft;
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
    if (random(2) == 0) voice.trigger(VOICE_PROFILE);
  }
  voice.control();
}

int updateAudio() { return voice.audio(); }

void setup() {
  const unsigned long seed = analogRead(0);
  randomSeed(seed);
  voice = ara::BirdVoice(seed);
  voice.setRegisterRange(MIN_REGISTER_SEMITONES, MAX_REGISTER_SEMITONES);
  voice.trigger(VOICE_PROFILE);
  scheduleAttempt(millis());
  startMozzi(CONTROL_RATE_HZ);
}

void loop() { audioHook(); }
