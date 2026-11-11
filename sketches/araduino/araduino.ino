#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>

constexpr int CONTROL_RATE_HZ = 64;
constexpr long MIN_ATTEMPT_INTERVAL_MS = 170000;
constexpr long MAX_ATTEMPT_INTERVAL_MS = 370000;

// C4 to B4
const int NOTES[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494}; 

const int SCALES[][8] = {
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

const float MELODIES[][8] = {
  {0.1, 0.2, 0.1, 0.1, 0.1, 0.2, 0.0, 0.0},
  {0.8, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.6, 0.2, 0.4, 0.2, 0.0, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.4, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.4, 0.2, 0.2, 0.2, 0.0, 0.0},
  {0.2, 0.8, 0.2, 0.4, 0.0, 0.0, 0.0, 0.0},
  {0.2, 0.8, 0.2, 0.4, 0.2, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.2, 0.8, 0.0, 0.0, 0.0, 0.0},
  {0.2, 0.4, 0.4, 0.2, 0.6, 0.0, 0.0, 0.0},
  {0.6, 0.2, 0.4, 0.2, 0.4, 0.0, 0.0, 0.0},
  {0.6, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.4, 0.4, 0.2, 0.2, 0.4, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.2, 0.2, 0.4, 0.6, 0.0, 0.0},
  {0.6, 0.2, 0.2, 0.4, 0.2, 0.0, 0.0, 0.0},
  {0.2, 0.2, 0.2, 0.2, 0.2, 0.4, 0.0, 0.0},
  {0.2, 0.2, 0.2, 0.2, 0.2, 0.4, 0.0, 0.0},
  {0.6, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0}
};

Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscil(SIN2048_DATA);

bool isPlaying = false;
bool shouldRepeatMelody = false;
unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;
unsigned long lastAttemptTime = 0;
unsigned long attemptInterval = 0;
int currentMelodyIndex = 0;
int currentNoteIndex = 0;
int currentKey = 0;
int currentScaleIndex = 0;
int chordNotes[5];

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
  if (currentNoteIndex >= 8 || MELODIES[currentMelodyIndex][currentNoteIndex] == 0) {
    if (shouldRepeatMelody) {
      currentNoteIndex = 0;
      shouldRepeatMelody = false;
    } else {
      isPlaying = false;
      return;
    }
  }

  if (MELODIES[currentMelodyIndex][currentNoteIndex] > 0) {
    int note = NOTES[chordNotes[random(5)]] * 4;
    int octave = random(1, 4);
    noteDuration = MELODIES[currentMelodyIndex][currentNoteIndex] * 1000;

    note *= octave;
    note += random(-10, 12);

    aOscil.setFreq(note);

    // Set overlapping note start (e.g., start the next note slightly earlier)
    int overlapDuration = noteDuration * 0.65;
    noteStartTime = millis();
    isPlaying = true;
    noteDuration = overlapDuration;
  }

  currentNoteIndex++;
}

void startNewSong() {
  currentKey = random(12);
  currentScaleIndex = random(7);
  currentMelodyIndex = random(16);
  currentNoteIndex = 0;

  shouldRepeatMelody = random(1, 7) > 3;

  chordNotes[0] = currentKey;
  chordNotes[1] = (currentKey + SCALES[currentScaleIndex][2]) % 12;
  chordNotes[2] = (currentKey + SCALES[currentScaleIndex][4]) % 12;
  chordNotes[3] = (currentKey + SCALES[currentScaleIndex][6]) % 12;
  chordNotes[4] = (currentKey + SCALES[currentScaleIndex][8]) % 12;

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
