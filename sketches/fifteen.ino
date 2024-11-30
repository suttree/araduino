#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <RTClib.h> // Include the RTC library

RTC_DS3231 rtc;

#define CONTROL_RATE 64

// Sunrise and Sunset Times (precomputed for the 15th of each month, London)
float sunriseTimes[12] = {8.05, 7.25, 6.30, 5.50, 5.05, 4.50, 5.00, 5.35, 6.20, 7.05, 7.50, 8.20};
float sunsetTimes[12] = {16.05, 17.10, 18.00, 19.10, 20.10, 21.00, 20.50, 20.00, 19.00, 17.40, 16.30, 15.50};

// Musical constants
const int NOTES[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494}; // C4 to B4
const int SCALES[][8] = {
  {0, 2, 4, 5, 7, 9, 11, 12},   // Major
  {0, 2, 3, 5, 7, 8, 10, 12},   // Minor (Natural)
  {0, 2, 3, 5, 7, 9, 11, 12},   // Melodic Minor
  {0, 2, 4, 5, 7, 9, 10, 12},   // Mixolydian
  {0, 2, 3, 5, 7, 8, 10, 12},   // Dorian
  {0, 1, 3, 5, 7, 8, 10, 12},   // Phrygian
  {0, 2, 4, 6, 7, 9, 11, 12},   // Lydian
  {0, 1, 3, 5, 6, 8, 10, 12},   // Locrian
  {0, 2, 4, 6, 8, 10, 12, 14},  // Whole Tone
  {0, 3, 5, 7, 10, 12, 15, 17}, // Minor Pentatonic
  {0, 4, 7, 12, 16, 19, 24, 28}, // Major Pentatonic
  {0, 2, 3, 5, 6, 8, 9, 12},    // Hungarian Minor
  {0, 2, 3, 6, 7, 8, 11, 12},   // Neapolitan Minor
  {0, 1, 4, 5, 7, 8, 10, 12},   // Neapolitan Major
  {0, 3, 4, 7, 9, 12, 15, 16},  // Arabian
  {0, 2, 3, 6, 7, 9, 10, 12},   // Harmonic Minor
  {0, 2, 4, 7, 9, 11, 13, 14},  // Double Harmonic Major
  {0, 2, 5, 7, 8, 10, 12, 14}   // Japanese
};

// Melody patterns
const float MELODIES[][8] = {
  {0.8, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.6, 0.2, 0.4, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.4, 0.0, 0.0, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.4, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.2, 0.8, 0.2, 0.4, 0.2, 0.0, 0.0, 0.0},
  {0.2, 0.8, 0.2, 0.4, 0.2, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.2, 0.8, 0.0, 0.0, 0.0, 0.0},
  {0.2, 0.4, 0.4, 0.2, 0.6, 0.0, 0.0, 0.0},
  {0.6, 0.2, 0.4, 0.2, 0.4, 0.0, 0.0, 0.0},
  {0.6, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.4, 0.4, 0.2, 0.2, 0.4, 0.0, 0.0, 0.0},
  {0.4, 0.2, 0.2, 0.2, 0.4, 0.6, 0.0, 0.0},
  {0.8, 0.2, 0.2, 0.4, 0.2, 0.0, 0.0, 0.0},
  {0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.2, 0.2, 0.2, 0.2, 0.2, 0.4, 0.0, 0.0},
  {0.6, 0.1, 0.1, 0.2, 0.2, 0.0, 0.0, 0.0}
};

// Mozzi oscillator
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscil(SIN2048_DATA);

// State variables
bool isPlaying = false;
bool shouldRepeatMelody = false;
unsigned long noteStartTime = 0;
unsigned long noteDuration = 0;
unsigned long lastCheckTime = 0;
int currentMelodyIndex = 0;
int currentNoteIndex = 0;
int currentKey = 0;
int currentScaleIndex = 0;
int chordNotes[5];

// Helper functions for time calculations
int convertTimeToMinutes(float time) {
  int hours = (int)time;
  int minutes = (time - hours) * 60;
  return hours * 60 + minutes;
}

bool isWithinWindow(int currentMinutes, int targetMinutes, int windowMinutes) {
  return abs(currentMinutes - targetMinutes) <= windowMinutes;
}

float smoothNoise(float x) {
  int xi = (int)x; // Integer part of x
  float xf = x - xi; // Fractional part of x

  // Hashing function for pseudo-random repeatable values
  int hash = (xi * 73 + 97) % 256; 
  float left = (hash % 100) / 100.0; // Noise value at x = xi
  hash = ((xi + 1) * 73 + 97) % 256;
  float right = (hash % 100) / 100.0; // Noise value at x = xi+1

  // Smooth interpolation
  return left + xf * (right - left);
}

float noiseOffset = 0; // Offset to smoothly vary the noise

bool shouldSing() {
  // Check if current time is near sunrise or sunset
  DateTime now = rtc.now();
  int currentMinutes = now.hour() * 60 + now.minute();
  int month = now.month();

  int sunriseMinutes = convertTimeToMinutes(sunriseTimes[month - 1]);
  int sunsetMinutes = convertTimeToMinutes(sunsetTimes[month - 1]);

  bool nearSunrise = isWithinWindow(currentMinutes, sunriseMinutes, 15);
  bool nearSunset = isWithinWindow(currentMinutes, sunsetMinutes, 15);

  noiseOffset += 0.05; // Adjust increment for slower/faster variation

  float noiseValue = smoothNoise(noiseOffset);
  int mappedValue = map(noiseValue * 100, 0, 100, 1, 7);

    mappedValue = sunsetMinutes;
  
  return mappedValue >= 5;
}

void playNextNote() {
  if (currentNoteIndex >= 8 || MELODIES[currentMelodyIndex][currentNoteIndex] == 0) {
    if (shouldRepeatMelody) {
      // Restart the melody if the flag is true
      currentNoteIndex = 0;
      shouldRepeatMelody = false;
    } else {
      // Stop playing if no repeat
      isPlaying = false;
      return;
    }
  }

  if (MELODIES[currentMelodyIndex][currentNoteIndex] > 0) {
    int note = NOTES[chordNotes[random(5)]] * 4; // Shift up
    int octave = random(1, 4);
    noteDuration = MELODIES[currentMelodyIndex][currentNoteIndex] * 1000;

    // Apply octave and random transposition
    note *= octave;
    note += random(-10, 12); // Subtle detuning for a more natural sound

    // Set the frequency for the oscillator
    aOscil.setFreq(note);

    // Set overlapping note start (e.g., start the next note slightly earlier)
    int overlapDuration = noteDuration * 0.65; // 65% of the duration for overlap
    noteStartTime = millis();
    isPlaying = true;
    noteDuration = overlapDuration;  // Adjust the note duration to allow overlap
  }

  currentNoteIndex++;
}

void startNewSong() {
  currentKey = random(12);
  currentScaleIndex = random(3);
  currentMelodyIndex = random(8);
  currentNoteIndex = 0;
  
  // Determine if melody should repeat based on random number
  shouldRepeatMelody = random(1, 7) > 3;
  
  // Generate chord progression
  chordNotes[0] = currentKey;
  chordNotes[1] = (currentKey + SCALES[currentScaleIndex][2]) % 12;
  chordNotes[2] = (currentKey + SCALES[currentScaleIndex][4]) % 12;
  chordNotes[3] = (currentKey + SCALES[currentScaleIndex][6]) % 12;
  chordNotes[4] = (currentKey + SCALES[currentScaleIndex][8]) % 12;
  
  Serial.println("Starting new song...");
  playNextNote();
}

void setup() {
  Serial.begin(9600);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    //while (1); // Stop if the RTC is not found
  } else {
    //Serial.println("Ok\r\n");
  }
  startMozzi(CONTROL_RATE);
  randomSeed(analogRead(0));
}

void updateControl() {
  unsigned long currentTime = millis();

  // Get current time from RTC
  DateTime now = rtc.now();
  int currentHour = now.hour();
  // Only play between 9 AM and 10 PM (22:00)
  if (currentHour >= 9 && currentHour < 22) {
    // Check if we should start a new song
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
  } else {
    // Stop playing outside allowed hours
    //Serial.println("Nope");
    isPlaying = false;
  }
}

int updateAudio() {
  return isPlaying ? aOscil.next() : 0;
}

void loop() {
  audioHook();
}