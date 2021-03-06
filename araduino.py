import os, sys, getopt, datetime
import random

def usage():
  print "araduino.py -h (help) -d (debug) -s (startup)"

def main(argv):
  try:
    opts, args = getopt.getopt(argv, "hdsm", ['help', 'debug', 'startup', 'morning'])
  except getopt.GetoptError:
    usage()
    sys.exit(2)

  global _debug
  _debug = 0

  morning = False 

  for opt, arg in opts:
    if opt in ("-h", "--help"):
      usage()
      sys.exit()

    if opt in ("-m", "--morning"):
      morning = True
      _debug = 1

    if opt == '-d':
      _debug = 1

    if opt in ("-s", "--startup"):
      import time
      time.sleep(90)
      #os.system("/usr/bin/tvservice -o")


  # Increase chance of singing at sunrise/sunset
  import ephem

  birdcage = ephem.Observer()
  birdcage.lat = '51.497517'
  birdcage.lon = '0.080380'
  birdcage.date = str(datetime.datetime.now())
  birdcage.elevation = 5

  sun = ephem.Sun()

  next_sunrise = birdcage.next_rising(sun)
  early_next_sunrise = ephem.Date(next_sunrise - 15 * ephem.minute) 
  late_next_sunrise = ephem.Date(next_sunrise + 15 * ephem.minute) 

  next_sunset = birdcage.next_setting(sun)
  early_next_sunset = ephem.Date(next_sunset - 15 * ephem.minute) 
  late_next_sunset = ephem.Date(next_sunset + 15 * ephem.minute) 

  sunrise = False;
  sunset = False;
  if (birdcage.date > early_next_sunrise and birdcage.date < late_next_sunrise):
    #print 'Sunrise roll'
    sunrise = true;
    dice_roll = random.choice([1,2,3,4,5,6,7,8])
  elif (birdcage.date > early_next_sunset and birdcage.date < late_next_sunset):
    #print 'Sunset roll'
    sunset = true;
    dice_roll = random.choice([1,2,3,4,5,6,7,8])
  else:
    dice_roll = random.choice([1,2,3,4,5,6])

  if (dice_roll < 5 and _debug <> 1):
    #print "Going back to sleep"
    sys.exit()

  # We're alive, import what else we need now
  sys.path.append(os.path.join(os.path.dirname(__file__), 'python-musical'))

  from musical.theory import Note, Scale, Chord
  from musical.audio import effect, playback

  from timeline import Hit, Timeline

  # Define key and scale
  key = Note((random.choice(Note.NOTES), random.choice([2,3,3])))

  scales = ['major', 'minor', 'melodicminor', 'harmonicminor', 'pentatonicmajor', 'bluesmajor', 'pentatonicminor', 'bluesminor', 'augmented', 'diminished', 'wholehalf', 'halfwhole', 'augmentedfifth', 'japanese', 'oriental', 'ionian', 'phrygian', 'lydian', 'mixolydian', 'aeolian', 'locrian']
  random.shuffle(scales)
  scale = Scale(key, random.choice(scales))

  #print key
  #print scale

  # Grab progression chords from scale starting at the octave of our key
  progression = Chord.progression(scale, base_octave=key.octave)

  time = 0.0 # Keep track of correct note placement time in seconds

  timeline = Timeline()

  # Pick a notes from a chord randomly chosen from a list of notes in this progression
  chord = progression[ random.choice(range(len(progression)-1)) ]
  notes = chord.notes

  melodies = [ 
    [0.8, 0.2],
    [0.4, 0.2],
    [0.2, 0.8],
    [0.2, 0.4],
    [0.6, 0.2],
    [0.4, 0.4, 0.2],
    [0.6, 0.1, 0.1],
    [0.8, 0.1, 0.2],
    [0.2, 0.2, 0.2],
    [0.2, 0.4, 0.2],
    [1.0, 0.1, 0.2, 0.1, 0.2, 0.10, 0.1],
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

  if sunrise or sunset:
    random_melody = random.choice(melodies[0:12])
  else:
    random_melody = random.choice(melodies)

  # Testing a new melody-generation idea - duncan 11/4/20
  # - needs more work, disabling for now - 12/4/20
  #random_melody = []
  #melody_length = random.randrange(1, 12)
  #
  #for i in range(0, melody_length):
  #  random_melody.append( round(random.uniform(0.1, 0.6), 1) )
  # test end

  if morning:
    random_melody = melodies[-1]

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
      random_transpose = random.choice([0,2,4,6,8,10,12])

    last_interval = interval
    last_transpose = random_transpose

    note = random_note.transpose(random_transpose)
    #print note

    # favour queued notes, but occasionally overlap them too
    if (random.choice([1,2,3,4,5,6]) > 2):
      time = time + interval
      timeline.add(time, Hit(note, interval))
    else:
      timeline.add(time, Hit(note, interval))
      time = time + interval

  #print "Rendering audio..."
  data = timeline.render()

  # Reduce volume to 50%
  data = data * 0.5

  print "Playing audio..."
  if morning:
    for i in range(2):
      playback.play(data)
  else:
    for i in range(random.choice([1,2])):
      playback.play(data)

  #print "Done!"

if __name__ == "__main__":
  main(sys.argv[1:])
