#include <TimeLib.h>
#include <Tone.h>

// Pin definitions
const int SPEAKER_PIN = 3;

// Location settings (London, UK)
const float LATITUDE = 51.497517;
const float LONGITUDE = 0.080380;

// Time settings
const int GMT_OFFSET = 0; // GMT timezone

// Tone generator
Tone toneGenerator;

// Musical constants
const int NOTES[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494}; // C4 to B4
const int SCALES[][8] = {
  {0, 2, 4, 5, 7, 9, 11, 12}, // Major
  {0, 2, 3, 5, 7, 8, 10, 12}, // Minor
  {0, 2, 3, 5, 7, 9, 11, 12}  // Melodic Minor
};

// Melody patterns (simplified version of your Python melodies)
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

void setup() {
  Serial.begin(9600);
  
  // Initialize the tone generator
  toneGenerator.begin(SPEAKER_PIN);
  
  // Set the time (you would typically sync this with a real-time clock module)
  setTime(12, 0, 0, 14, 10, 2024); // 12:00:00 14 Oct 2024
  
  randomSeed(analogRead(0)); // Initialize random number generator
}

void loop() {
  time_t t = now();
  
  Serial.print("Regular message");
  
  // Check if it's around sunrise or sunset
  bool isSunriseOrSunset = isAroundSunriseOrSunset(t);
  
  // Determine if we should sing
  if (shouldSing(isSunriseOrSunset)) {
    sing(isSunriseOrSunset);
  }
  
  delay(60000); // Check every minute
}

bool isAroundSunriseOrSunset(time_t t) {
  // This is a simplified calculation and may not be accurate
  int hours = hour(t);
  int minutes = minute(t);
  
  // Assume sunrise is around 6 AM and sunset is around 6 PM
  return (hours == 5 && minutes >= 45) || (hours == 6 && minutes <= 15) ||
         (hours == 17 && minutes >= 45) || (hours == 18 && minutes <= 15);
}

bool shouldSing(bool isSunriseOrSunset) {
  int roll = random(1, 7); // Roll a 6-sided die
  
  if (isSunriseOrSunset) {
    return roll <= 4; // 66% chance during sunrise/sunset
  } else {
    return roll == 1; // 16% chance otherwise
  }
}

void sing(bool isSunriseOrSunset) {
  // Choose a random key and scale
  int key = random(12);
  int scaleIndex = random(3);
  
  // Choose a random melody pattern
  int melodyIndex = isSunriseOrSunset ? random(5) : random(8);
  
  // Generate chord progression (simplified)
  int chordNotes[3] = {key, (key + SCALES[scaleIndex][2]) % 12, (key + SCALES[scaleIndex][4]) % 12};
  
  Serial.println("Singing...");
  
  for (int i = 0; i < 8; i++) {
    if (MELODIES[melodyIndex][i] > 0) {
      int note = NOTES[chordNotes[random(3)]];
      int octave = random(1, 4); // Random octave (1-3)
      int duration = MELODIES[melodyIndex][i] * 1000; // Convert to milliseconds
      
      // Apply octave and random transposition
      note *= octave;
      note += random(-50, 51); // Random transposition within a quarter-tone
      
      toneGenerator.play(note, duration);
      delay(duration);
      toneGenerator.stop();
      
      delay(50); // Short pause between notes
    }
  }
}
