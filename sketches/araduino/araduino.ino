#include <MozziGuts.h>
#include <Oscil.h>
#include <mozzi_pgmspace.h>
#include <tables/sin2048_int8.h>

constexpr int CONTROL_RATE_HZ = 64;
constexpr long MIN_ATTEMPT_INTERVAL_MS = 170000;
constexpr long MAX_ATTEMPT_INTERVAL_MS = 370000;

// C4 to B4
CONSTTABLE_STORAGE(uint16_t NOTES[]) = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};
constexpr uint8_t NOTE_COUNT = sizeof(NOTES) / sizeof(NOTES[0]);

CONSTTABLE_STORAGE(uint8_t SCALES[][8]) = {
  {0, 2, 4, 5, 7, 9, 11, 12},   // Major
  {0, 2, 3, 5, 7, 9, 11, 12},   // Melodic Minor
  {0, 2, 4, 5, 7, 9, 10, 12},   // Mixolydian
  {0, 2, 4, 7, 9, 11, 13, 14},  // Double Harmonic Major
  {0, 3, 4, 7, 9, 12, 15, 16},  // Arabian
  {0, 1, 4, 5, 7, 8, 10, 12},   // Neapolitan Major
  {0, 1, 3, 5, 6, 8, 10, 12},   // Locrian
  {0, 2, 3, 5, 7, 8, 10, 12},   // Dorian
  {0, 2, 5, 7, 8, 10, 12, 14}   // Japanese
};
constexpr uint8_t SCALE_COUNT = sizeof(SCALES) / sizeof(SCALES[0]);

// Note lengths in milliseconds. A zero ends the line.
constexpr uint8_t MAX_LINE_NOTES = 8;
CONSTTABLE_STORAGE(uint16_t MELODY_DURATIONS[][MAX_LINE_NOTES]) = {
  {100, 200, 100, 100, 100, 200},
  {800, 200, 200, 200, 200},
  {600, 200, 400, 200},
  {400, 200, 400},
  {400, 200, 400, 200, 200, 200},
  {200, 800, 200, 400},
  {200, 800, 200, 400, 200},
  {400, 200, 200, 800},
  {200, 400, 400, 200, 600},
  {600, 200, 400, 200, 400},
  {600, 200, 200, 200, 200},
  {400, 400, 200, 200, 400},
  {400, 200, 200, 200, 400, 600},
  {600, 200, 200, 400, 200},
  {200, 200, 200, 200, 200, 400},
  {200, 200, 200, 200, 200, 400},
  {600, 200, 200, 200, 200}
};
constexpr uint8_t MELODY_COUNT = sizeof(MELODY_DURATIONS) / sizeof(MELODY_DURATIONS[0]);
constexpr unsigned long NOTE_LENGTH_PERCENT = 65;

// Root, third, fifth, seventh, and ninth. The ninth uses the second degree's pitch class.
CONSTTABLE_STORAGE(uint8_t CHORD_DEGREES[]) = {0, 2, 4, 6, 1};
constexpr uint8_t CHORD_NOTE_COUNT = sizeof(CHORD_DEGREES) / sizeof(CHORD_DEGREES[0]);

Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscil(SIN2048_DATA);

bool isPlaying = false;
bool shouldRepeatMelody = false;
unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;
unsigned long lastAttemptTime = 0;
unsigned long attemptInterval = 0;
uint8_t currentMelodyIndex = 0;
uint8_t currentNoteIndex = 0;
uint8_t chordNotes[CHORD_NOTE_COUNT];

void scheduleNextAttempt(unsigned long now) {
  lastAttemptTime = now;
  attemptInterval = random(MIN_ATTEMPT_INTERVAL_MS, MAX_ATTEMPT_INTERVAL_MS + 1);
}

void updateControl() {
  const unsigned long now = millis();
  if (isPlaying) {
    if (now - noteStartTime >= noteDuration) {
      playNextNote();
    }
  } else if (now - lastAttemptTime >= attemptInterval) {
    scheduleNextAttempt(now);
    if (random(2) == 0) {
      startNewSong();
    }
  }
}

void playNextNote() {
  uint16_t duration = currentNoteIndex < MAX_LINE_NOTES
    ? FLASH_OR_RAM_READ(&MELODY_DURATIONS[currentMelodyIndex][currentNoteIndex])
    : 0;
  if (duration == 0) {
    if (shouldRepeatMelody) {
      currentNoteIndex = 0;
      shouldRepeatMelody = false;
      duration = FLASH_OR_RAM_READ(&MELODY_DURATIONS[currentMelodyIndex][0]);
    } else {
      isPlaying = false;
      return;
    }
  }

  int frequency = FLASH_OR_RAM_READ(&NOTES[chordNotes[random(CHORD_NOTE_COUNT)]]) * 4;
  frequency *= random(1, 4);
  frequency += random(-10, 12);
  aOscil.setFreq(frequency);

  noteStartTime = millis();
  noteDuration = duration * NOTE_LENGTH_PERCENT / 100;
  isPlaying = true;
  currentNoteIndex++;
}

void startNewSong() {
  const uint8_t key = random(NOTE_COUNT);
  const uint8_t scale = random(SCALE_COUNT);
  currentMelodyIndex = random(MELODY_COUNT);
  currentNoteIndex = 0;

  shouldRepeatMelody = random(2) == 0;

  for (uint8_t i = 0; i < CHORD_NOTE_COUNT; i++) {
    const uint8_t degree = FLASH_OR_RAM_READ(&CHORD_DEGREES[i]);
    const uint8_t offset = FLASH_OR_RAM_READ(&SCALES[scale][degree]);
    chordNotes[i] = (key + offset) % NOTE_COUNT;
  }
  playNextNote();
}

void setup() {
  randomSeed(analogRead(0));
  startMozzi(CONTROL_RATE_HZ);
  startNewSong();
  scheduleNextAttempt(millis());
}

const float volume = 0.5;

int updateAudio() {
    return isPlaying ? aOscil.next() * volume : 0;
}

void loop() {
  audioHook();
}
