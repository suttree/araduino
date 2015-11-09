import os, sys, getopt
import random

def usage():
  print "araduino.py -h (help) -d (debug)"

def main(argv):
  try:
    opts, args = getopt.getopt(argv, "hd", ['help', 'debug'])
  except getopt.GetoptError:
    usage()
    sys.exit(2)

  global _debug
  _debug = 0
  for opt, arg in opts:
    if opt in ("-h", "--help"):
      usage()
      sys.exit()
    elif opt == '-d':
      _debug = 1

  dice_roll = random.choice([1,2,3,4,5,6])
  if (dice_roll < 6 and _debug <> 1):
    sys.exit()

  sys.path.append(os.path.join(os.path.dirname(__file__), 'python-musical'))

  from musical.theory import Note, Scale, Chord
  from musical.audio import effect, playback

  from timeline import Hit, Timeline

  # Define key and scale
  #key = Note(random.choice(Note.NOTES))
  key = Note((random.choice(Note.NOTES), random.choice([0,1,2,3])))

  scales = ['major', 'minor', 'melodicminor', 'harmonicminor', 'pentatonicmajor', 'bluesmajor', 'pentatonicminor', 'bluesminor', 'augmented', 'diminished', 'wholehalf', 'halfwhole', 'augmentedfifth', 'japanese', 'oriental', 'ionian', 'dorian', 'phrygian', 'lydian', 'mixolydian', 'aeolian', 'locrian']
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

  # Pick a notes from a chord randomly chosen from a list of notes in this progression
  chord = progression[ random.choice(range(len(progression)-1)) ]
  notes = chord.notes

  melodies = [
    [0.8, 0.4, 0.4, 0.2, 0.2],
    [0.2, 0.2, 0.4, 0.4, 0.8, 0.8],
    [1.0, 0.4, 0.4, 0.4, 0.8, 0.8],
    [0.2, 0.4, 0.2, 0.4, 0.2],
    [0.4, 0.4, 0.4, 0.8, 0.8, 1.0, 1.0],
    [0.2, 0.2, 0.4, 0.4, 0.8],
    [0.4, 0.0, 0.4, 0.0, 0.8, 1.0, 0.2],
  ]

  random_melody = random.choice(melodies)
  print random_melody

  for i, interval in enumerate(random_melody):
    random_note = random.choice(notes)
    random_transpose = random.choice([0, 12, 14])

    note = random_note.transpose(random_transpose)

    time = time + interval
    timeline.add(time, Hit(note, interval))

  # Strum out root chord to finish
  timeline.add(time, Hit(chord.notes[1], 1.0))
  timeline.add(time, Hit(chord.notes[2].transpose(12), 1.0))

  print "Rendering audio..."

  data = timeline.render()

  #data = effect.chorus(data, freq=3.14159)
  #data = effect.tremolo(data, freq=3.14159)
  #data = effect.flanger(data, freq=3.14159)

  # Reduce volume to 25%
  data = data * 0.25

  print "Playing audio..."

  playback.play(data)

  print "Done!"

if __name__ == "__main__":
  main(sys.argv[1:])
