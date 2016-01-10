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

  # Increase change of singing at sunrise/sunset
  import ephem

  birdcage = ephem.Observer()
  birdcage.lat = '51.5034070'
  birdcage.lon = '-0.1275920'
  birdcage.elevation = 19

  sun = ephem.Sun()

  next_sunrise = birdcage.next_rising(sun)
  early_next_sunrise = ephem.Date(next_sunrise - 15 * ephem.minute) 
  late_next_sunrise = ephem.Date(next_sunrise + 15 * ephem.minute) 

  next_sunset = birdcage.next_setting(sun)
  early_next_sunset = ephem.Date(next_sunset - 15 * ephem.minute) 
  late_next_sunset = ephem.Date(next_sunset + 15 * ephem.minute) 

  if (birdcage.date > early_next_sunrise and birdcage.late_< next_sunrise):
    print 'Sunrise roll'
    dice_roll = random.choice([1,2,3,4,5])
  elif (birdcage.date > early_next_sunset and birdcage.date < late_next_sunset):
    print 'Sunset roll'
    dice_roll = random.choice([1,2,3,4,5])
  else:
    dice_roll = random.choice([1,2,3,4,5,6])

  if (dice_roll < 5 and _debug <> 1):
    print "Going back to sleep"
    sys.exit()

  # We're alive, import what else we need now
  sys.path.append(os.path.join(os.path.dirname(__file__), 'python-musical'))

  from musical.theory import Note, Scale, Chord
  from musical.audio import effect, playback

  from timeline import Hit, Timeline

  # Define key and scale
  key = Note((random.choice(Note.NOTES), random.choice([2,3,3])))

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
    [1.0, 0.1, 0.2, 0.1, 0.2, 0.10, 0.1],
    [0.8, 0.1, 0.1, 0.2],
    [0.8, 0.4, 0.1, 0.2, 0.4, 0.1, 0.2],
    [0.8, 0.4, 0.4, 0.2, 0.2, 0.1, 0.1],
    [0.4, 0.0, 0.1, 0.1, 0.2, 0, 0.1, 0.4],
    [0.1, 0.1, 0.1, 0.0, 0.2, 0.0, 0.1, 0.2, 0.4],
    [0.8, 0.4, 0.1, 0.4, 0.2, 0.2, 0.1, 0.2, 0.8, 0.1, 0.4, 0.1],
    [0.2, 0.2, 0.4, 0.2, 0.1, 0.1, 0.0, 0.2],
    [1.0, 0.1, 0.2, 0.1, 0.2, 0.2],
    [0.2, 0.1, 0.2, 0.4, 0.1, 0.2, 0.4],
    [0.4, 0.1, 0.4, 0.2, 0.4, 0.1, 0.4, 0.2],
    [0.1, 0.1, 0.1, 0.2, 0.1, 0.1, 0.2],
    [0.1, 0.1, 0.1, 0.2, 0.1, 0.1, 0.1, 0.2, 0.0],
    [0.1, 0.0, 0.1, 0.0, 0.1, 0.0, 0.2, 0.0, 0.2, 0.0, 0.1, 0.1, 0.3],
  ]

  random_melody = random.choice(melodies)
  print random_melody

  last_interval = 0.0
  last_transpose = 0

  for i, interval in enumerate(random_melody):
    random_note = random.choice(notes)

    # the first note should be high
    # identical intervals should often hold the same pitch
    # otherwise, pick a random pitch
    if i == 0:
      random_transpose = random.choice([8, 12])
    elif (last_interval == interval):
      if random.choice([0,1,2]) == 2:
        random_transpose = last_transpose
      else:
        random_transpose = 0
    else:
      random_transpose = random.choice([0, 2,4,6,8,10,12])

    last_interval = interval
    last_transpose = random_transpose

    note = random_note.transpose(random_transpose)
    #print note

    # favour queued notes, but occasionally overlap them too
    if (random.choice([1,2,3,4,5,6]) > 4):
      time = time + interval
      timeline.add(time, Hit(note, interval))
    else:
      timeline.add(time, Hit(note, interval))
      time = time + interval

  print "Rendering audio..."
  data = timeline.render()

  # Reduce volume to 95%
  data = data * 0.95

  print "Playing audio..."
  for i in range(random.choice([1,2])):
    playback.play(data)

  print "Done!"

if __name__ == "__main__":
  main(sys.argv[1:])
