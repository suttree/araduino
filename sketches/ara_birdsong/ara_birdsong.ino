#include <MozziGuts.h>
#include "BirdVoice.h"

static_assert(AUDIO_RATE == 16384, "This voice requires 16384 Hz audio.");
static_assert(MOZZI_AUDIO_BIAS == 244, "High gain requires the default Uno PWM output range.");
static_assert(MOZZI_PWM_RATE == 32768, "High gain requires the default Uno PWM carrier rate.");
constexpr uint16_t CONTROL_RATE_HZ = 256;
constexpr unsigned long MIN_SING_ATTEMPT_INTERVAL_MS = 9UL * 60UL * 1000UL;
constexpr unsigned long MAX_SING_ATTEMPT_INTERVAL_MS = 180UL * 60UL * 1000UL;
// 100 is the original preview level. Use the Uno's wider PWM output range.
constexpr uint16_t OUTPUT_GAIN_PERCENT = 350;
// Disable the experimental noisy onset while testing physical playback.
constexpr bool NOISY_ONSETS = false;
// +2, +4 or +6 above the previous +11 upper register, weighted 40%, 50%, 10%.
constexpr uint8_t LOWER_REGISTER_SEMITONES = 13;
constexpr uint8_t MIDDLE_REGISTER_SEMITONES = 15;
constexpr uint8_t UPPER_REGISTER_SEMITONES = 17;
constexpr uint8_t LOWER_REGISTER_WEIGHT = 4;
constexpr uint8_t MIDDLE_REGISTER_WEIGHT = 5;
constexpr uint8_t UPPER_REGISTER_WEIGHT = 1;
ara::BirdVoice voice;
unsigned long lastAttempt = 0;
unsigned long attemptInterval = 0;

void scheduleAttempt(unsigned long now) {
  lastAttempt = now;
  attemptInterval = random(MIN_SING_ATTEMPT_INTERVAL_MS, MAX_SING_ATTEMPT_INTERVAL_MS + 1UL);
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
  // Mix several analogue readings and their timing, rather than one pin value.
  unsigned long seed = micros();
  for (uint8_t i = 0; i < 16; ++i) {
    seed ^= analogRead(0);
    seed ^= micros();
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
  }
  randomSeed(seed);
  voice = ara::BirdVoice(seed);
  voice.setGainPercent(OUTPUT_GAIN_PERCENT);
  voice.setNoisyOnsets(NOISY_ONSETS);
  voice.setRegisterChoices(LOWER_REGISTER_SEMITONES, MIDDLE_REGISTER_SEMITONES,
      UPPER_REGISTER_SEMITONES, LOWER_REGISTER_WEIGHT, MIDDLE_REGISTER_WEIGHT, UPPER_REGISTER_WEIGHT);
  voice.triggerRandom();
  scheduleAttempt(millis());
  startMozzi(CONTROL_RATE_HZ);
}

void loop() { audioHook(); }
