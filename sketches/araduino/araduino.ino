#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include <RTClib.h>

RTC_DS3231 rtc;

#define CONTROL_RATE 64

// Sunrise and Sunset Times (precomputed for the 15th of each month, London)
float sunriseTimes[12] = {8.05, 7.25, 6.30, 5.50, 5.05, 4.50, 5.00, 5.35, 6.20, 7.05, 7.50, 8.20};
float sunsetTimes[12] = {16.05, 17.10, 18.00, 19.10, 20.10, 21.00, 20.50, 20.00, 19.00, 17.40, 16.30, 15.50};

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
  {0.6, 0.2, 0.2, 0.4, 0.2, 0.0, 0.0, 0.0},
  {0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0},
  {0.2, 0.2, 0.2, 0.2, 0.2, 0.4, 0.0, 0.0},
  {0.6, 0.1, 0.1, 0.2, 0.2, 0.0, 0.0, 0.0}
};

Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscil(SIN2048_DATA);

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

int convertTimeToMinutes(float time) {
  int hours = (int)time;
  int minutes = (time - hours) * 60;
  return hours * 60 + minutes;
}

bool isWithinWindow(int _currentMinutes, int targetMinutes, int windowMinutes) {
  return abs(_currentMinutes - targetMinutes) <= windowMinutes;
}

/*
float smoothNoise(float x) {
  int xi = (int)x;
  float xf = x - xi;

  // Hashing function for pseudo-random repeatable values
  int hash = (xi * 73 + 97) % 256; 
  float left = (hash % 100) / 100.0;
  hash = ((xi + 1) * 73 + 97) % 256;
  float right = (hash % 100) / 100.0;

  return left + xf * (right - left);
}

float noiseOffset = 0;

bool shouldSing() {
  noiseOffset += 0.05; // Adjust increment for slower/faster variation

  float noiseValue = smoothNoise(noiseOffset);
  int mappedValue = map(noiseValue * 100, 0, 100, 1, 7);

  return mappedValue >= 5;
}
*/

/*
bool shouldSing() {
  noiseOffset += variationSpeed;

  // Generate Perlin noise value between 0 and 1
  float noiseValue = (sin(noiseOffset) + 1.0) / 2.0; // Simulating Perlin with sin for simplicity

  // Add randomness for more natural variation
  float randomNoise = (rand() % 100) / 100.0 * randomFactor;

  // Threshold for singing
  return (noiseValue + randomNoise) > 0.6; // Adjust threshold as needed
}
*/

bool shouldSing() {
  return random(1, 7) >= 4;
}

#include <math.h>

float noiseOffset = 0;
float variationSpeed = 0.03; // Adjust for slower/faster variation
float randomFactor = 0.2;   // Introduces slight randomness for natural behavior

const unsigned long baseInterval = 190000;
const unsigned long noiseRange = 100000;

unsigned long calculateDynamicInterval() {
    long adjustment = random(-noiseRange, noiseRange + 1);
    return baseInterval + adjustment;
}

bool nearSunrise = false;
bool nearSunset = false;

void updateControl() {
  unsigned long currentTime = millis();

  DateTime now = rtc.now();
  int currentHour = now.hour();
  int currentMinutes = now.hour() * 60 + now.minute();
  int month = now.month();

  int sunriseMinutes = convertTimeToMinutes(sunriseTimes[month - 1]);
  nearSunrise = abs(currentMinutes - sunriseMinutes) <= 30;

  static unsigned long lastInterval = 0;
  unsigned long adjustedInterval = calculateDynamicInterval();

 if (nearSunrise || nearSunset) {
    //Serial.println("SUNRISE OR SUNSET");
    //Serial.println(currentTime);
    //Serial.println(currentHour);
    //Serial.println(currentMinutes);
    //Serial.println(sunriseMinutes);
    //Serial.println(currentMinutes - sunriseMinutes);
    //Serial.println(sunriseTimes[month - 1]);
    if (!isPlaying && (currentTime - lastCheckTime >= adjustedInterval)) {
      lastCheckTime = currentTime;
      if (shouldSing()) {
        startNewSong();
      }
    }
  } else if (currentHour >= 9 && currentHour <= 22) {
    //Serial.println("HERE TO GO");
    //Serial.println(currentTime);
    //Serial.println(currentHour);
    //Serial.println(lastCheckTime);
    //Serial.println(adjustedInterval);
    if (!isPlaying && (currentTime - lastCheckTime >= adjustedInterval)) {
      //Serial.println("WOAH");
      lastCheckTime = currentTime;
      if (shouldSing()) {
        startNewSong();
      }
    }
  } else {
    //Serial.println(currentHour);
    //Serial.println("HERE TO SKIP");
    isPlaying = false;
  }

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

  Serial.println("Starting new song...");
  playNextNote();
}

void setup() {
  Serial.begin(9600);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  startMozzi(CONTROL_RATE);
  randomSeed(analogRead(0));
}

const float volume = 0.5;

int updateAudio() {
    return isPlaying ? aOscil.next() * volume : 0;
}

void loop() {
  audioHook();
}
