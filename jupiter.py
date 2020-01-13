#   # to explore
#   https://github.com/wybiral/python-musical/blob/master/examples/example-01.py
#
#   # Define key and scale
#   key = Note('D3')
#   scale = Scale(key, 'minor')
#
#   # Grab progression chords from scale starting at the octave of our key
#   progression = Chord.progression(scale, base_octave=key.octave)
#
#   time = 0.0 # Keep track of currect note placement time in seconds
#
#   timeline = Timeline()
#
#   # Add progression to timeline by arpeggiating chords from the progression
#   for index in [0, 2, 3, 1,    0, 2, 3, 4,    5, 4, 0]:
#       chord = progression[index]
#       root, third, fifth = chord.notes
#       arpeggio = [root, third, fifth, third, root, third, fifth, third]
#       for i, interval in enumerate(arpeggio):
#           ts = float(i * 2) / len(arpeggio)
#           timeline.add(time + ts, Hit(interval, 1.0))
#       time += 2.0
#
#   # Strum out root chord to finish
#   chord = progression[0]
#   timeline.add(time + 0.0, Hit(chord.notes[0], 4.0))
#   timeline.add(time + 0.1, Hit(chord.notes[1], 4.0))
#   timeline.add(time + 0.2, Hit(chord.notes[2], 4.0))
#   timeline.add(time + 0.3, Hit(chord.notes[1].transpose(12), 4.0))
#   timeline.add(time + 0.4, Hit(chord.notes[2].transpose(12), 4.0))
#   timeline.add(time + 0.5, Hit(chord.notes[0].transpose(12), 4.0))
#
#   ---
#
#   https://github.com/wybiral/python-musical/blob/master/examples/example-02.py
#   # Resolve
#   note = scale.transpose(key, random.choice((-1, 1, 4)))
#   timeline.add(time, Hit(note, 0.75))     # Tension
#   timeline.add(time + 0.5, Hit(key, 4.0)) # Resolution
#
#   print("Rendering audio...")
#
#   data = timeline.render()
#
#   print("Applying chorus effect...")
#
#   data = effect.chorus(data, freq=3.14159)
#
#   ---
#
#   https://github.com/wybiral/python-musical/blob/216d2211f06746d540e048be795947a33ddff747/musical/audio/effect.py#L28
#   effects ^^
#
#   --
#
#   https://github.com/wybiral/python-musical/blob/master/examples/example-03.py
#
#   # Define key and scale
#   key = Note('C4')
#   scale = Scale(key, 'major')
#
#   note = key
#   chunks = []
#   for i in range(len(scale)):
#       third = scale.transpose(note, 2)
#       chunks.append(source.sine(note, 0.5) + source.square(third, 0.5))
#       note = scale.transpose(note, 1)
#   fifth = scale.transpose(key, 4)
#   chunks.append(source.sine(key, 1.5) + source.square(fifth, 1.5))
#
#   print("Rendering audio...")
#
#   data = numpy.concatenate(chunks)
#
#   --
#
#   Can I pick a chord, then transpose it to 'major' and 'minor' depending on time of day?
#       chord.py + major/minor/augmented/diminished
#           returns triads, which we can use for songs
#       
#       
#   Use ringbuffer and and pluck for other effects?

#----------

import os, sys, getopt, datetime
import random

def main(argv):
  global _debug
  _debug = 1

  morning = False 

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
    dice_roll = random.choice([1,2,3,4,5,6,7,8,9,10])
  elif (birdcage.date > early_next_sunset and birdcage.date < late_next_sunset):
    #print 'Sunset roll'
    sunset = true;
    dice_roll = random.choice([1,2,3,4,5,6,7,8,9,10])
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

  scales = ['major', 'minor', 'melodicminor', 'harmonicminor', 'pentatonicmajor', 
          'bluesmajor', 'pentatonicminor', 'bluesminor', 'augmented', 'diminished', 
          'wholehalf', 'halfwhole', 'augmentedfifth', 'japanese', 'oriental', 'ionian', 
          'phrygian', 'lydian', 'mixolydian', 'aeolian', 'locrian']
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

  # Reduce volume to 95%
  data = data * 0.65
  
  print "Playing audio..."
  if morning:
    for i in range(2):
      playback.play(data)
  else:
    for i in range(random.choice([1,2])):
      playback.play(data)
      

if __name__ == "__main__":
  main(sys.argv[1:])