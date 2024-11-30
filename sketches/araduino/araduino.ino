// Need to add in more melodies and scales

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
  {0, 2, 3, 5, 7, 9, 11, 12},   // Melodic Minor
  {0, 2, 4, 5, 7, 9, 10, 12},    // Mixolydian
  {0, 2, 4, 7, 9, 11, 13, 14},  // Double Harmonic Major
  {0, 3, 4, 7, 9, 12, 15, 16},  // Arabian
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
  noiseOffset += 0.05; // Adjust increment for slower/faster variation

  float noiseValue = smoothNoise(noiseOffset);
  int mappedValue = map(noiseValue * 100, 0, 100, 1, 7);

  return mappedValue >= 5;
}

const unsigned long baseInterval = 190000; // Base interval: 3 minutes 10 seconds (in ms)
const unsigned long noiseRange = 20000;    // Noise range: ±20 seconds (in ms)

// Function to calculate a dynamic interval with noise
unsigned long calculateDynamicInterval() {
    // Generate a random adjustment within the noise range
    long adjustment = random(-noiseRange, noiseRange + 1); // Random value between -20000 and +20000
    return baseInterval + adjustment;
}

// Flag to indicate whether it's near sunrise or sunset
bool nearSunrise = false;
bool nearSunset = false;

void updateControl() {
  unsigned long currentTime = millis();

  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMinutes = now.hour() * 60 + now.minute();
  int month = now.month();

  // Convert sunrise and sunset times to minutes for comparison
  int sunriseMinutes = convertTimeToMinutes(sunriseTimes[month - 1]);
  int sunsetMinutes = convertTimeToMinutes(sunsetTimes[month - 1]);

  //const unsigned long baseInterval = 190000; // Base interval: 2 minutes 10 seconds (in ms)
  //const unsigned long noiseRange = 20000;    // Noise range: ±20 seconds (in ms)

  static unsigned long lastInterval = 0; // Store the last interval
  unsigned long adjustedInterval = calculateDynamicInterval(); // Get a new dynamic interval
  //unsigned long adjustedInterval = baseInterval + random(-noiseRange, noiseRange + 1);

 if (nearSunrise || nearSunset) {
    if (!isPlaying && (currentTime - lastCheckTime >= adjustedInterval)) {
      lastCheckTime = currentTime;
      if (shouldSing()) {
        startNewSong();
      }
    }
  } else if (currentHour >= 9 && currentHour <= 22) {
    if (!isPlaying && (currentTime - lastCheckTime >= adjustedInterval)) {
      lastCheckTime = currentTime;
      if (shouldSing()) {
        startNewSong();
      }
    }
  } else {
    isPlaying = false;
  }

  //unsigned long currentTime = millis();
  // Run the song and update it only if it's within the singing window
  if (isPlaying && (currentTime - noteStartTime >= noteDuration)) {
    playNextNote();
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
    int note = NOTES[chordNotes[random(5)]] * 4; // Shift up
    int octave = random(1, 4);
    noteDuration = MELODIES[currentMelodyIndex][currentNoteIndex] * 1000;

    note *= octave;
    note += random(-10, 12); // Subtle detuning for natural sound

    aOscil.setFreq(note);

    noteStartTime = millis();
    isPlaying = true;
  }

  currentNoteIndex++;
}

void startNewSong() {
  currentKey = random(12);
  currentScaleIndex = random(7);
  currentMelodyIndex = random(16); // Updated to match the size of MELODIES
  currentNoteIndex = 0;

  shouldRepeatMelody = random(1, 7) > 3;

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
    while (1);
  }

  startMozzi(CONTROL_RATE);
  randomSeed(analogRead(0));
}

const float volume = 0.5; // Adjust volume (0.0 = silent, 1.0 = full volume)

int updateAudio() {
    return isPlaying ? aOscil.next() * volume : 0;
}

void loop() {
  audioHook();
}
