# to explore
https://github.com/wybiral/python-musical/blob/master/examples/example-01.py

# Define key and scale
key = Note('D3')
scale = Scale(key, 'minor')

# Grab progression chords from scale starting at the octave of our key
progression = Chord.progression(scale, base_octave=key.octave)

time = 0.0 # Keep track of currect note placement time in seconds

timeline = Timeline()

# Add progression to timeline by arpeggiating chords from the progression
for index in [0, 2, 3, 1,    0, 2, 3, 4,    5, 4, 0]:
    chord = progression[index]
    root, third, fifth = chord.notes
    arpeggio = [root, third, fifth, third, root, third, fifth, third]
    for i, interval in enumerate(arpeggio):
        ts = float(i * 2) / len(arpeggio)
        timeline.add(time + ts, Hit(interval, 1.0))
    time += 2.0

# Strum out root chord to finish
chord = progression[0]
timeline.add(time + 0.0, Hit(chord.notes[0], 4.0))
timeline.add(time + 0.1, Hit(chord.notes[1], 4.0))
timeline.add(time + 0.2, Hit(chord.notes[2], 4.0))
timeline.add(time + 0.3, Hit(chord.notes[1].transpose(12), 4.0))
timeline.add(time + 0.4, Hit(chord.notes[2].transpose(12), 4.0))
timeline.add(time + 0.5, Hit(chord.notes[0].transpose(12), 4.0))

---

https://github.com/wybiral/python-musical/blob/master/examples/example-02.py
# Resolve
note = scale.transpose(key, random.choice((-1, 1, 4)))
timeline.add(time, Hit(note, 0.75))     # Tension
timeline.add(time + 0.5, Hit(key, 4.0)) # Resolution

print("Rendering audio...")

data = timeline.render()

print("Applying chorus effect...")

data = effect.chorus(data, freq=3.14159)

---

https://github.com/wybiral/python-musical/blob/216d2211f06746d540e048be795947a33ddff747/musical/audio/effect.py#L28
effects ^^

--

https://github.com/wybiral/python-musical/blob/master/examples/example-03.py

# Define key and scale
key = Note('C4')
scale = Scale(key, 'major')

note = key
chunks = []
for i in range(len(scale)):
    third = scale.transpose(note, 2)
    chunks.append(source.sine(note, 0.5) + source.square(third, 0.5))
    note = scale.transpose(note, 1)
fifth = scale.transpose(key, 4)
chunks.append(source.sine(key, 1.5) + source.square(fifth, 1.5))

print("Rendering audio...")

data = numpy.concatenate(chunks)
