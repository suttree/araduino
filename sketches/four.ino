// Define audio mode for Mozzi
#define AUDIO_MODE STANDARD
#define AUDIO_OUTPUT_PIN 3

#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>  // Sine wave table

// Set up a Mozzi oscillator using a sine wave table
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscil(SIN2048_DATA);

void setup() {
  Serial.begin(9600);
  startMozzi();                   // Start Mozzi
  aOscil.setFreq(440);            // Set oscillator frequency to 440 Hz (A4)
}

void updateControl() {
  // No control updates needed for this basic test
}

int updateAudio() {
  return aOscil.next();           // Output the oscillator’s next sample
}

void loop() {
  audioHook();                    // Required for Mozzi to handle audio
}
