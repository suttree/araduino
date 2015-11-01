import random

import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__), 'python-musical'))

from musical.theory import Note, Scale, Chord
from musical.audio import playback

from timeline import Hit, Timeline

# Define key and scale
key = Note(random.choice(Note.NOTES))

scales = ['major', 'minor', 'melodicminor', 'harmonicminor', 'pentatonicmajor', 'bluesmajor', 'pentatonicminor', 'bluesminor', 'augmented', 'diminished', 'chromatic', 'wholehalf', 'halfwhole', 'wholetone', 'augmentedfifth', 'japanese', 'oriental', 'ionian', 'dorian', 'phrygian', 'lydian', 'mixolydian', 'aeolian', 'locrian']
scale = Scale(key, random.choice(scales))

print key
print scale

# Grab progression chords from scale starting at the octave of our key
progression = Chord.progression(scale, base_octave=key.octave)

time = 0.0 # Keep track of currect note placement time in seconds

timeline = Timeline()

# Add progression to timeline by arpeggiating chords from the progression
#for index in [0, 2, 3, 1]:
#  chord = progression[index]
#  root, third, fifth = chord.notes
#  arpeggio = [root, third, fifth, third]
#  for i, interval in enumerate(arpeggio):
#    ts = float(i * 2) / len(arpeggio)
#    timeline.add(time + ts, Hit(interval, 1.0))
#  time += 2.0

# Strum out root chord to finish
chord = progression[0]

notes = [0, 1, 2]

#for i, interval in enumerate([0.2, 0.4, 0.6, 0.8, 0.10]):
for _ in range(5):
  random_duration = random.choice([0.2, 0.4, 0.6])
  random_note = random.choice(notes)
  random_transpose = random.choice([0, 12, 24])

  note = chord.notes[random_note].transpose(random_transpose)

  timeline.add(time + random_duration, Hit(note, 2))

print "Rendering audio..."

data = timeline.render()

# Reduce volume to 25%
data = data * 0.25

print "Playing audio..."

playback.play(data)

print "Done!"
