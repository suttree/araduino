import random

import sys
sys.path.append('./python-musical')

from musical.theory import Note, Scale, Chord
from musical.audio import playback

from timeline import Hit, Timeline

# Define key and scale
key = Note(random.choice(Note.NOTES))
scale = Scale(key, random.choice(['major', 'minor']))

print key

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

for i, interval in enumerate([0.2, 0.4, 0.6, 0.8, 0.10]):
  random_note = random.choice(notes)
  random_transpose = random.choice([0, 12, 24])

  note = chord.notes[random_note].transpose(random_transpose)

  timeline.add(time + interval, Hit(note, random.choice([1,2,3,4])))

print "Rendering audio..."

data = timeline.render()

# Reduce volume to 25%
data = data * 0.25

print "Playing audio..."

playback.play(data)

print "Done!"
