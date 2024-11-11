#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>

#define CONTROL_RATE 64

// Musical constants
const int NOTES[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494}; // C4 to B4
const int SCALES[][8] = {
  {0, 2, 4, 5, 7, 9, 11, 12},  // Major
  {0, 2, 3, 5, 7, 8, 10, 12},  // Minor
  {0, 2, 3, 5, 7, 9, 11, 12}   // Melodic Minor
};

// Melody patterns
const float MELODIES[][8] = {
  {0.8, 0.2, 0.2, 0.2, 0.2, 0.8, 0.0, 0.0},
  {0.4, 0.2, 0.4, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.2, 0.8, 0.8, 0.8, 0.2, 0.0, 0.0, 0.0},
  {0.2, 0.4, 0.4, 0.2, 0.6, 0.0, 0.0, 0.0},
  {0.6, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.4, 0.4, 0.2, 0.2, 0.4, 0.6, 0.0, 0.0},
  {0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.8, 0.1, 0.1, 0.2, 0.2, 0.2, 0.2, 0.2}
};

// Mozzi oscillator
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscil(SIN2048_DATA);

// State variables
bool isPlaying = false;
unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;
unsigned long lastCheckTime = 0;
int currentMelodyIndex = 0;
int currentNoteIndex = 0;
int currentKey = 0;
int currentScaleIndex = 0;
int chordNotes[3];

bool shouldSing() {
  return random(1, 7) >= 4;
}

void playNextNote() {
  if (currentNoteIndex >= 8 || MELODIES[currentMelodyIndex][currentNoteIndex] == 0) {
    isPlaying = false;
    return;
  }
  
  if (MELODIES[currentMelodyIndex][currentNoteIndex] > 0) {
    int note = NOTES[chordNotes[random(3)]] * 4; // Shift up by two octaves
    int octave = random(1, 4);
    noteDuration = MELODIES[currentMelodyIndex][currentNoteIndex] * 1000;
    
    // Apply octave and random transposition
    note *= octave;
    note += random(-50, 51);
    
    // Set the frequency for the oscillator
    aOscil.setFreq(note);
    
    noteStartTime = millis();
    isPlaying = true;
  }
  
  currentNoteIndex++;
}

void startNewSong() {
  currentKey = random(12);
  currentScaleIndex = random(3);
  currentMelodyIndex = random(8);
  currentNoteIndex = 0;
  
  // Generate chord progression
  chordNotes[0] = currentKey;
  chordNotes[1] = (currentKey + SCALES[currentScaleIndex][2]) % 12;
  chordNotes[2] = (currentKey + SCALES[currentScaleIndex][4]) % 12;
  
  Serial.println("Starting new song...");
  playNextNote();
}

void setup() {
  Serial.begin(9600);
  startMozzi(CONTROL_RATE);
  randomSeed(analogRead(0));
}

void updateControl() {
  unsigned long currentTime = millis();
  
  // Check if we should start a new song every minute
  if (!isPlaying && (currentTime - lastCheckTime >= 60000)) {
    lastCheckTime = currentTime;
    if (shouldSing()) {
      startNewSong();
    }
  }
  
  // If we are playing, check if it's time for the next note
  if (isPlaying && (currentTime - noteStartTime >= noteDuration)) {
    playNextNote();
  }
}

int updateAudio() {
  return isPlaying ? aOscil.next() : 0;
}

void loop() {
  audioHook(); // Required for Mozzi
}
