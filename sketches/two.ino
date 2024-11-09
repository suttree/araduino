// Define audio mode for pin 3 BEFORE including Mozzi headers
#define AUDIO_MODE STANDARD_PLUS // Required for non-standard pins
#define AUDIO_OUTPUT_PIN 3

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <TimeLib.h>

// Rest of the code remains exactly the same as before
#define CONTROL_RATE 64

// Location settings (London, UK)
const float LATITUDE = 51.497517;
const float LONGITUDE = 0.080380;

// Time settings
const int GMT_OFFSET = 0; // GMT timezone

// Musical constants
const int NOTES[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494}; // C4 to B4
const int SCALES[][8] = {
  {0, 2, 4, 5, 7, 9, 11, 12},  // Major
  {0, 2, 3, 5, 7, 8, 10, 12},  // Minor
  {0, 2, 3, 5, 7, 9, 11, 12}   // Melodic Minor
};

// Melody patterns
const float MELODIES[][8] = {
  {0.8, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.4, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.2, 0.8, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.2, 0.4, 0.4, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.6, 0.2, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.4, 0.4, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.8, 0.1, 0.1, 0.2, 0.2, 0.2, 0.2, 0.2}
};

// Mozzi oscillator
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscil(SIN2048_DATA);

// State variables
bool isPlaying = false;
unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;
int currentMelodyIndex = 0;
int currentNoteIndex = 0;
int currentKey = 0;
int currentScaleIndex = 0;
int chordNotes[3];

bool isAroundSunriseOrSunset(time_t t) {
  int hours = hour(t);
  int minutes = minute(t);
  return (hours == 5 && minutes >= 45) || 
         (hours == 6 && minutes <= 15) || 
         (hours == 17 && minutes >= 45) || 
         (hours == 18 && minutes <= 15);
}

bool shouldSing(bool isSunriseOrSunset) {
  int roll = random(1, 7); // Roll a 6-sided die
  return isSunriseOrSunset ? (roll <= 4) : (roll == 1);
}

void playNextNote() {
  if (currentNoteIndex >= 8 || MELODIES[currentMelodyIndex][currentNoteIndex] == 0) {
    isPlaying = false;
    return;
  }
  
  if (MELODIES[currentMelodyIndex][currentNoteIndex] > 0) {
    int note = NOTES[chordNotes[random(3)]];
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

void startNewSong(bool isSunriseOrSunset) {
  currentKey = random(12);
  currentScaleIndex = random(3);
  currentMelodyIndex = isSunriseOrSunset ? random(5) : random(8);
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
  
  // Set the time (you would typically sync this with a real-time clock module)
  setTime(12, 0, 0, 14, 10, 2024); // 12:00:00 14 Oct 2024
}

void updateControl() {
  // This function runs at CONTROL_RATE frequency
  
  time_t t = now();
  bool isSunriseOrSunset = isAroundSunriseOrSunset(t);
  
  // If we're not playing and it's time to sing
  if (!isPlaying && shouldSing(isSunriseOrSunset)) {
    startNewSong(isSunriseOrSunset);
  }
  
  // If we are playing, check if it's time for the next note
  if (isPlaying && (millis() - noteStartTime >= noteDuration)) {
    playNextNote();
  }
}

int updateAudio() {
  // This function runs at AUDIO_RATE frequency
  return isPlaying ? aOscil.next() : 0;
}

void loop() {
  audioHook(); // Required for Mozzi
}