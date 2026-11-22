#include <MozziGuts.h>
#include "BirdVoice.h"

static_assert(AUDIO_RATE == 16384, "This voice requires 16384 Hz audio.");
constexpr uint16_t CONTROL_RATE_HZ = 256;
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
    if (random(2) == 0) voice.trigger(ara::BirdVoice::Soft);
  }
  voice.control();
}

int updateAudio() { return voice.audio(); }

void setup() {
  const unsigned long seed = analogRead(0);
  randomSeed(seed);
  voice = ara::BirdVoice(seed);
  voice.trigger(ara::BirdVoice::Soft);
  scheduleAttempt(millis());
  startMozzi(CONTROL_RATE_HZ);
}

void loop() { audioHook(); }
