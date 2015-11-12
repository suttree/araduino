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
  key = Note((random.choice(Note.NOTES), random.choice([2])))

  scales = ['major', 'minor', 'melodicminor', 'harmonicminor', 'pentatonicmajor', 'bluesmajor', 'pentatonicminor', 'bluesminor', 'augmented', 'diminished', 'wholehalf', 'halfwhole', 'augmentedfifth', 'japanese', 'oriental', 'ionian', 'phrygian', 'lydian', 'mixolydian', 'aeolian', 'locrian']
  scale = Scale(key, random.choice(scales))

  print key
  print scale

  # Grab progression chords from scale starting at the octave of our key
  progression = Chord.progression(scale, base_octave=key.octave)

  time = 0.0 # Keep track of currect note placement time in seconds

  timeline = Timeline()

  # Pick a notes from a chord randomly chosen from a list of notes in this progression
  chord = progression[ random.choice(range(len(progression)-1)) ]
  notes = chord.notes

  melodies = [
    [1.0, 0.1, 0.1, 0.1, 0.1, 0.1, 0.10, 0.1],
    [0.8, 0.1, 0.1, 0.1, 0.2],
    [0.8, 0.4, 0.4, 0.2, 0.2],
    [0.4, 0.0, 0.1, 0.1, 0.2, 0, 0.1, 0.4],
    [0.1, 0.1, 0.1, 0.0, 0.2, 0.0, 0.1, 0.2, 0.4],
    [0.8, 0.4, 0.4, 0.2, 0.2, 0.2, 0.8, 0.4, 0.4, 0.2, 0.2, 0.2],
    [0.2, 0.2, 0.4, 0.4, 0.2, 0.1, 0.0, 0.2, 0.4],
    [1.0, 0.4, 0.4, 0.4, 0.2],
    [0.2, 0.2, 0.4, 0.4, 0.8],
    [0.4, 0.4, 0.2, 0.4, 0.4, 0.2],
    [0.1, 0.1, 0.1, 0.2, 0.1, 0.1, 0.1, 0.2],
    [0.1, 0.1, 0.1, 0.2, 0.1, 0.1, 0.1, 0.2, 0.0, 0.1, 0.1, 0.1, 0.2, 0.1, 0.1, 0.1, 0.2],
    [0.1, 0.0, 0.1, 0.0, 0.1, 0.0, 0.2, 0.0, 0.2, 0.0, 0.1, 0.0, 0.1, 0.0, 0.1, 0.0, 0.3],
  ]

  random_melody = random.choice(melodies)
  print random_melody

  last_interval = 0.0

  for i, interval in enumerate(random_melody):
    random_note = random.choice(notes)

    # more note repitition
    if (last_interval == interval):
      random_transpose = 0
    else:
      random_transpose = random.choice([0, 0, 0, 12])

    last_interval = interval

    note = random_note.transpose(random_transpose)

    timeline.add(time, Hit(note, interval))
    time = time + interval

  print "Rendering audio..."
  data = timeline.render()

  # Reduce volume to 50%
  data = data * 0.50

  print "Playing audio..."
  for i in range(random.choice([1,2])):
    playback.play(data)

  print "Done!"

if __name__ == "__main__":
  main(sys.argv[1:])
