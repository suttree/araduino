# Birdsong voice experiment

This sketch experiments with shaped whistles and repeated calls on the existing Uno hardware. The working sketch at `../araduino/araduino.ino` remains separate.

## Shorter waits, fewer high songs and melody repeats, 27 November

Ara now draws a fresh wait uniformly between 9 and 25 minutes before each attempt to sing, keeping the 50% chance of singing. The average gap between songs is 34 minutes. Skipped attempts can make the gap longer than 25 minutes. She still sings once on startup.

The register weights are now 44% lower, 54% middle and 2% highest. The highest register was previously a 10% choice. Gain remains 350%.

Each song has a 60% chance of repeating its first melody exactly, including note order, pitch, rhythm, trills, ending and held notes. The other 40% generates a fresh second melody in the same voice and register, avoiding the last three contours. Every song still contains two phrases. The earlier answering-phrase transformations have been removed. A held note in a repeated melody now occurs in both phrases; a new second melody chooses its own held-note gesture.

The [new demo reel](../../output/birdsong-session-20261127/ara-melody-session-ef25b4205a5c.mp3) contains ten examples over approximately 58 seconds, with two seconds between songs. Five repeat exactly and five play a new second melody. The first three examples repeat; the fourth, starting around 0:18.4, introduces a new second melody around 0:21.9. This seed contains only lower and middle registers. The small sample does not measure the 60% repeat probability or 2% highest-register probability. The adjacent `melody-session.csv` records repeat choices and second-phrase start times. Reproduce it with:

```sh
python3 tools/audio/render.py --session --seed 20261127 --output output/birdsong-session-20261127
```

The [second reel](../../output/birdsong-session-20261127-reel2/ara-melody-session-c607294ce083.mp3) contains ten fresh examples over approximately 57 seconds. Six repeat exactly and four play a new second melody. The opening example changes melody around 0:02.8; the next example starts around 0:06.8 and repeats around 0:09.2. This seed also contains only lower and middle registers. Reproduce it with:

```sh
python3 tools/audio/render.py --session --seed 2026112702 --output output/birdsong-session-20261127-reel2
```

This revision has not been uploaded to Ara. Earlier recordings below retain their original settings.

## Random waits of 9 to 30 minutes, 26 November

Ara now draws a fresh wait uniformly between 9 and 30 minutes before each attempt to sing. Each attempt has a 50% chance of starting a song. After every attempt, including a skipped one, she draws another wait. She still sings once on startup.

The average wait between attempts is 19.5 minutes. With the 50% singing chance, the average gap between songs is 39 minutes. Skipped attempts can make the gap longer than 30 minutes.

Firmware from commit `590f51b` was uploaded to Ara on 26 November 2026. avrdude verified all 10162 bytes by flash readback. Physical listening to this upload remains unconfirmed.

## Random waits of 9 to 60 minutes, 25 November

Ara now draws a fresh wait uniformly between 9 and 60 minutes before each attempt to sing. Each attempt has a 50% chance of starting a song. After every attempt, including a skipped one, she draws another wait. She still sings once on startup.

The average wait between attempts is 34.5 minutes. With the 50% singing chance, the average gap between songs is 69 minutes. Skipped attempts can make the gap longer than 60 minutes.

Firmware from commit `db4b82d` was uploaded to Ara on 25 November 2026. avrdude verified all 10162 bytes by flash readback. Physical listening to this upload remains unconfirmed.

## Random waits of 9 to 120 minutes, 25 November

Ara now draws a fresh wait uniformly between 9 and 120 minutes before each attempt to sing. Each attempt still has a 50% chance of starting a song. After every attempt, including a skipped one, she draws another wait. She still sings once on startup.

The average wait between attempts is 64.5 minutes. With the 50% singing chance, the average gap between songs is 129 minutes, or two hours and nine minutes. Skipped attempts can make the gap longer than 120 minutes.

Firmware from commit `e6f30b7` was uploaded to Ara on 25 November 2026. avrdude verified all 10162 bytes by flash readback. Physical listening to this upload remains unconfirmed.

## A chance to sing every 18 minutes, 25 November

Ara now makes one attempt every 18 minutes, with a 50% chance of singing at each attempt. If she skips one, she waits another 18 minutes before trying again. This gives an average of one song every 36 minutes, with longer gaps possible. She still sings once on startup.

Firmware from commit `8e0969d` was uploaded to Ara on 25 November 2026. avrdude verified all 10128 bytes by flash readback. Physical listening to this upload remains unconfirmed.

## Less frequent singing, 24 November

Ara now waits 340 to 740 seconds between attempts to sing, twice the previous 170 to 370 seconds. Each attempt still has a 50% chance of starting a song. This gives an average of one song every 18 minutes, previously nine minutes. The minimum interval is five minutes and 40 seconds; skipped attempts can produce much longer gaps. She still sings once on startup. Song generation and volume remain unchanged.

Firmware from commit `8b95b54` was uploaded to Ara on 24 November 2026. avrdude verified all 10162 bytes by flash readback. Physical listening to this upload remains unconfirmed.

## Shorter held notes and held openings, 24 November

Held notes now last 112 to 168 control ticks, approximately 0.44 to 0.66 seconds. This is a modest reduction from the previous 0.5 to 0.75 seconds. The chance of selecting a held note remains 30% for Song and TrillSong. One third of those selected songs begin with it. The rest place it later in the first phrase or in the answer. Each selected song still has exactly one held gesture.

The [fifth reel](../../output/birdsong-session-20261124-reel5/ara-melody-session-7dcc0e805ac6.mp3) contains ten examples over approximately 53 seconds. The last melody, around 0:49, opens with a held note of approximately 0.56 seconds. Other held notes occur around 0:05.8, 0:34.3 and 0:39. The seed was selected to include an opening example and later held gestures; this showcase does not measure their normal frequency. Gain, register choices, cadence and maximum of two renditions remain unchanged. Reproduce it with:

```sh
python3 tools/audio/render.py --session --seed 2026112406 --output output/birdsong-session-20261124-reel5
```

Firmware from commit `3ded42e` was uploaded to Ara on 24 November 2026. avrdude verified all 10162 bytes by flash readback. Physical listening to this upload remains unconfirmed.

## Occasional held notes, 24 November

The first held-note revision gave Song and TrillSong a 30% chance of including one held note, lasting 0.5 to 0.75 seconds. The latest revision shortens that range and deliberately includes opening gestures, as described above. Slight pitch drift and curvature, a softer level and longer attack and release remain. Trill pulses are suppressed for the held gesture. The remaining phrasing, 350% gain, register weights and two-rendition maximum are unchanged.

The [fourth demo reel](../../output/birdsong-session-20261124-reel4/ara-melody-session-e6083c057ce4.mp3) contains ten examples over approximately 55 seconds. Held notes begin around 0:08.9 and 0:24.1 and last approximately 0.74 and 0.61 seconds. The adjacent `melody-session.csv` records their exact locations and durations as well as song details. The demo uses the same illustrative profile order as the previous reels and a new seed:

```sh
python3 tools/audio/render.py --session --seed 2026112404 --output output/birdsong-session-20261124-reel4
```

This revision has not been uploaded to Ara.

## Looser cadence, 24 November

The owner identified a recurring short-short-long cadence in otherwise successful melodies. This revision removes the four fixed rhythm templates, fixed midpoint breath, predictable trill positions and matching opening/closing durations. Songs contain six to eight syllables, organised into irregular groups. Their gestures range from brief chirps to longer whistles and trills. Group boundaries have longer breaths; the answering phrase chooses its own syllable durations and breath positions rather than applying one tempo change to the whole rhythm.

Trills can open, interrupt or close a phrase. Endings can be brief, fading, trilled or left as the generated gesture. The answering phrase has a separate ending choice. Brief endings retain a short release ramp, so they do not simply switch the waveform off. The existing melody contours, trills, 350% gain, 40%/50%/10% register choices and maximum of two renditions remain.

The [third demo reel](../../output/birdsong-session-20261124-reel3/ara-melody-session-edab85a26172.mp3) showcases ten melody and trill examples over approximately 53 seconds. It is a melody demonstration, with a chosen profile order and generated note details, rather than a comparison of pitches. It uses seed `2026112403`, with two seconds between songs. The adjacent `melody-session.csv` records starts, durations, profiles and registers. Reproduce it with:

```sh
python3 tools/audio/render.py --session --seed 2026112403 --output output/birdsong-session-20261124-reel3
```

This revision has not been uploaded. Listening determines whether the cadence now feels less predictable.

## Melody-focused revision, 24 November

Gain is now 350%. No motif can play more than twice in one song. The register choices remain unchanged at 40% lower, 50% middle and 10% highest.

Random selection gives Song 45%, TrillSong 45%, Cascade 7%, and the earlier short-call profiles 3% combined. Repeat prevention tracks tunes rather than voice categories, allowing different melodic songs to follow each other. Twelve base contours can each play forward, backward, inverted, or backward and inverted, providing 48 melodic contour variations. The last three selected contours are avoided. Songs also vary rhythm, note shapes, trill placement and breaths. The second rendition can change its phrase order, tempo and pitch slightly; the latest revision above also changes its syllable-by-syllable rhythm. The earlier call profiles remain occasional choices; melody and trill songs now use six to eight syllables per rendition.

The [one-minute melody demo](../../output/birdsong-session-20261124/ara-melody-session-7b0cefecc223.mp3) showcases ten examples generated by the current engine at 350% gain, using the agreed register choices. The profile order was chosen to illustrate different melodies and trills, rather than to measure random selection probabilities. Register choices and all note details are generated from seed `20261124`. The normal waits between songs are shortened to two seconds. No pitch comparison, normalization, reverb or speaker simulation is applied.

| Start | Example |
| --- | --- |
| 0:00 | Melody |
| 0:06 | Melody and trills |
| 0:12.4 | Melody |
| 0:17.5 | Longer trill phrases |
| 0:24.6 | Melody |
| 0:30.3 | Cascade |
| 0:35.4 | Melody and trills |
| 0:41.8 | Melody |
| 0:47.9 | Melody and trills |
| 0:54.6 | Closing melody |

`output/birdsong-session-20261124/melody-session.csv` records the exact starts, durations, profiles, contour IDs, registers and maximum rendition count. To reproduce the demo:

```sh
python3 tools/audio/render.py --session
```

A [second reel](../../output/birdsong-session-20261124-reel2/ara-melody-session-0fa4c8d150ce.mp3) contains ten fresh examples over approximately 57 seconds. It uses the same gain, register weights, two-rendition limit and profile order, with seed `2026112402`. Its individual song details are in the adjacent `melody-session.csv`. Reproduce it with:

```sh
python3 tools/audio/render.py --session --seed 2026112402 --output output/birdsong-session-20261124-reel2
```

This revision is prepared for listening review and has not been uploaded to Ara.

## Less repetition, 23 November

This revision made the highest register a 10% choice, with 40% lower and 50% middle. It used 400% gain; the current gain is 350%.

The owner reported hearing the same two songs frequently. The earliest song profiles reused one eight-syllable pitch contour, fixed trill positions and three renditions. The 23 November revision introduced six contours, four rhythm patterns, variable phrase lengths, trill positions, sweep directions, note lengths and breaths. It allowed two or three renditions. The current revision above expands the contours and caps every song at two renditions.

That revision avoided consecutive voice categories. The current version instead avoids the last three tune contours, so melodic voices can follow each other without forcing an unrelated short call between them. Startup mixes sixteen analogue readings with their timing instead of seeding from one reading. This reduces reliance on a single input value; it does not guarantee different startup sequences.

The earlier audio samples below capture preceding versions. Rendering the current code produces the new phrasing.

## Songs and trills, 23 November

The first 400% song revision added three profiles built from an opening chirp, a motif, an answering phrase and trills. The recordings in this section use eight syllables and three renditions. The current generator varies that structure as described above. A song keeps one voice and one register throughout.

The [33-second comparison](../../output/birdsong-songs/ara-song-comparison-1aac9e961ad2.mp3) plays a preceding voice followed by the new profiles. All four use 400% gain, the same seed and a fixed register of eleven semitones above the original experiment:

| Start | Voice | Phrasing |
| --- | --- | --- |
| 0:00 | [Previous whistle](../../output/birdsong-songs/00-previous-whistle-cb566b9a9bd7.mp3) | A reference for the preceding Soft voice |
| 0:04.5 | [Motif song](../../output/birdsong-songs/01-motif-song-98523380e25e.mp3) | Opening chirps, repeated phrases and two trills per rendition |
| 0:14 | [Trill song](../../output/birdsong-songs/02-trill-song-327def474e36.mp3) | Four longer trills per rendition, with short answering chirps |
| 0:23.5 | [Cascade song](../../output/birdsong-songs/03-cascade-song-c67349b0aa51.mp3) | Descending chirps separated by trills and short breaths |

The rendered new songs last approximately 6.3, 8.0 and 5.2 seconds respectively. Song trills pulse almost to silence at 18 to 28 pulses per second, alongside pitch modulation. These profiles use a single sine oscillator, without the secondary tone or noisy onset of the earlier timbre experiments. The goal is a more recognisable song; naturalness still needs listening through Ara's speaker.

The first song selection pool favoured longer songs but still assigned 25% to earlier calls. The current selection is 90% melody/trill songs, 7% cascades and 3% earlier calls, as described above. Attempt intervals and the chance of remaining silent are unchanged.

The [random-song preview](../../output/birdsong-songs/04-random-song-mix-4ead8b466b3f.mp3) demonstrates the actual weighted selection and variable register, with the long pauses shortened. Its songs begin at 0:00, 0:13 and 0:26.

To reproduce these recordings:

```sh
python3 tools/audio/render.py --songs
```

These previews use seed `20261123`. Their PCM output maps the almost-nine-bit PWM range into 16-bit audio, so samples above 127 do not wrap. The comparison preserves the full first song in each section. No loudness normalization or reverb is applied. Earlier previews below retain their original settings.

## Listen

### Higher register samples, 23 November

The owner reported that the longer songs still sometimes sounded like a human whistle and requested higher-register samples. These recordings keep the song profiles, random seed, phrasing and 400% gain fixed. The first uses the top of the preceding firmware's range; the others raise it by another two, four and six semitones. The recordings were auditions and did not change the Arduino.

The [38-second comparison](../../output/birdsong-registers/ara-higher-register-comparison-1a4b429c15e7.mp3) repeats the same TrillSong at each pitch:

| Start | Register | Peak pitch across all three sample songs |
| --- | --- | --- |
| 0:00 | Previous upper register, +11 above the original experiment | 4718 Hz |
| 0:09.5 | [Two semitones higher](../../output/birdsong-registers/01-higher-plus13-258fffb48861.mp3) | 5295 Hz |
| 0:19 | [Four semitones higher](../../output/birdsong-registers/02-higher-plus15-525dd595ca4d.mp3) | 5943 Hz |
| 0:28.5 | [Six semitones higher](../../output/birdsong-registers/03-higher-plus17-536e5a100077.mp3) | 6672 Hz |

Each linked full sample contains Song at 0:00, TrillSong at 0:11 and Cascade at 0:22. They use seed `20261123`, with fixed pitch multipliers of 1.888, 2.119, 2.378 and 2.670. No sample reaches the engine's 7000 Hz pitch ceiling. The renderer rejects previews that hit that ceiling, to prevent flattened pitch contours from obscuring the comparison. Measurements are recorded in `output/birdsong-registers/register-peaks.csv`. A waveform cycle-count check also confirmed increasing pitches in the generated audio.

These previews use the constructor's fixed pitch multiplier. The owner subsequently requested all three higher registers as defaults, biased toward the upper choices. The configuration below implements that request. No speaker or enclosure response is simulated in the recordings.

To reproduce the samples:

```sh
python3 tools/audio/render.py --registers
```

The [comparison recording](../../output/birdsong/ara-voice-comparison.mp3) plays these approaches in order. Each section contains three bouts, with the long waits between songs shortened for auditioning.

| Start | Voice | Change |
| --- | --- | --- |
| 0:00 | [Current reference](../../output/birdsong/00-current-reference.mp3) | Existing sketch through a desktop oscillator adapter |
| 0:26.5 | [Soft whistles](../../output/birdsong/01-soft-whistles.mp3) | Longer whistles, slower volume contours and curved pitch sweeps |
| 0:53 | [Garden calls](../../output/birdsong/02-garden-calls.mp3) | Shorter syllables and a five-call motif |
| 1:19.5 | [Lively trills](../../output/birdsong/03-lively-trills.mp3) | Garden calls with pitch and volume flutter in selected syllables |

All new voices generate a motif, repeat it with a small pitch offset, then stop. They use frequencies measured in hertz rather than notes chosen from scales. Pitch and volume change throughout each syllable. The original profiles use a sine wave. The new variations below also try a second tone and a noisy onset. No profile uses recordings or added background audio.

The new voices now sing approximately eleven semitones higher than the first experiment, three semitones above the previous revision. Their pitch contours are multiplied by 1.888, with timing and volume preserved. The [register comparison](../../output/birdsong/soft-register-comparison.mp3) plays the original soft voice first, then the higher version at 0:26.5. Both use the same seed and phrases. The [original register](../../output/birdsong/04-soft-original-register.mp3) is also available separately. Passing `1000` as the second `BirdVoice` constructor argument selects that original register.

The [earlier short pitch comparison](../../output/birdsong/ara-register-steps-original-five-eight-5a592528fa73.mp3) plays the same phrase in its original register at 0:00, five semitones higher at 0:05, and eight semitones higher at 0:10. The [eight-semitone soft voice with a distinct filename](../../output/birdsong/ara-soft-plus-eight-semitones-b014f8cdde3f.mp3) remains available. A stale player cache is a possible explanation for hearing the same pitch, but has not been confirmed.

Run `python3 tools/audio/compare_registers.py` to recreate these files from the three saved Git revisions. This requires Git and ffmpeg. The script checks pitch ratios by counting waveform cycles with a threshold that excludes low-level encoding noise. The measured ratios are 1.3352 and 1.5862 relative to the original. It records results in `output/birdsong/register-verification.json` and gives each published audio file a filename containing its content hash.

After rendering the current voice, run `python3 tools/audio/compare_registers.py --current` for the [eight-versus-eleven comparison](../../output/birdsong/ara-register-steps-eight-eleven-a0ee560fcd89.mp3). The previous register plays at 0:00 and the new register at 0:05. The [new soft voice](../../output/birdsong/ara-soft-plus-eleven-semitones-0c4e28d480c3.mp3), [garden calls](../../output/birdsong/ara-garden-plus-eleven-semitones-84c231880d9a.mp3) and [lively trills](../../output/birdsong/ara-lively-plus-eleven-semitones-d11c2885fdfb.mp3) have distinct filenames. The measured pitch ratio is 1.8858 relative to the original, approximately 19% higher than the eight-semitone version.

## Register settings

The firmware chooses one of three registers per song. These constants near the top of `ara_birdsong.ino` set the offsets above the original experiment and their relative weights:

```cpp
constexpr uint8_t LOWER_REGISTER_SEMITONES = 13;
constexpr uint8_t MIDDLE_REGISTER_SEMITONES = 15;
constexpr uint8_t UPPER_REGISTER_SEMITONES = 17;
constexpr uint8_t LOWER_REGISTER_WEIGHT = 44;
constexpr uint8_t MIDDLE_REGISTER_WEIGHT = 54;
constexpr uint8_t UPPER_REGISTER_WEIGHT = 2;
```

These are the auditioned two, four and six semitones above the preceding +11 upper register. Their probabilities are now 44%, 54% and 2% respectively. Ara keeps the chosen register through the whole song and every repeat. `setRegisterChoices()` accepts the three registers and their relative weights; all-zero weights fall back to equal probabilities. The supported pitch table now covers 0 through 17 semitones. Out-of-range register values clamp to 17. The older `setRegisterRange()` method remains available for an equally weighted inclusive range.

Random motifs can exceed the pitch targets in the fixed auditions. The engine retains its 7000 Hz frequency limit; in the latest 200-song check it limited 831 of 191459 active control updates, approximately 0.4%. Those parts of the pitch contour flatten at the limit. This is not a claim of unlimited pitch headroom.

The firmware chooses a voice at random using the song weighting above. It retains Soft, Garden, Lively, Chirps, Liquid and Textured alongside Song, TrillSong and Cascade. The voice remains fixed through the song and its repeats. Consecutive songs can choose the same voice. To audition a fixed voice, replace the two `voice.triggerRandom()` calls with a call such as `voice.trigger(ara::BirdVoice::TrillSong)`.

`OUTPUT_GAIN_PERCENT` sets the firmware volume. It is now 350, applied before the output limit. The earlier 200% limit was imposed by the experiment's 8-bit sample return type, not by the full Uno PWM output range. The voice now returns a 16-bit sample and accepts gain settings up to 500. Output remains limited to -240 through +240, within Mozzi's default Uno range of -244 through +243. Peaks reaching that limit are clipped, so gain above that point can add distortion. Firmware assertions require the default PWM bias and carrier rate. The early desktop previews retain their historical gain settings; the latest melody session uses 350%.

[Mozzi's AVR documentation](https://sensorium.github.io/Mozzi/doc/html/hardware_avr.html) describes the almost-nine-bit output range. The bundled library confirms the 244 bias and direct addition of the sample to the PWM compare register. This provides additional digital headroom; physical loudness and distortion still require a listening test. The earlier recommendation that an amplifier was the only remaining option was premature.

`NOISY_ONSETS` is currently `false` in the firmware. The owner reported crackling only during songs after increasing the volume. This setting disables the experimental noise at the start of Textured syllables while preserving its second tone. Earlier audition recordings retain their original noise setting.

Disabling those onsets did not fully remove the crackle. Inspection of the AVR assembly then found signed 32-bit and 16-bit division helpers in the function that generates every audio sample. The sample function now uses explicit magnitude shifts and a precomputed gain multiplier. Its AVR assembly no longer calls division helpers. The owner confirmed that the crackle disappeared after uploading that change at 150% gain.

## Sound shape variations

The [20-second comparison](../../output/birdsong-shapes/ara-shape-comparison-d24496268783.mp3) contains these voices at the same fixed eleven-semitone register, in order:

| Start | Voice | Change |
| --- | --- | --- |
| 0:00 | [Whistle reference](../../output/birdsong-shapes/00-whistle-reference-0c4e28d480c3.mp3) | The preceding soft voice, unchanged |
| 0:05 | [Short chirps](../../output/birdsong-shapes/01-short-chirps-26b5459b54b4.mp3) | 55 to 109 ms calls, an early pitch peak, a falling tail and a decaying volume |
| 0:10 | [Liquid calls](../../output/birdsong-shapes/02-liquid-calls-0d4ece475f2f.mp3) | 94 to 184 ms calls with rapid pitch and volume movement and a quiet second tone |
| 0:15 | [Textured chirps](../../output/birdsong-shapes/03-textured-chirps-4f420806be12.mp3) | The same longer chirp shapes, a second tone and a brief noisy onset |

All four use the same random seed and output scale. The three alternatives use a different sequence of random choices from the reference. The Liquid and Textured profiles share their underlying syllable durations and pitch targets. The second tone runs at three quarters of the main tone's frequency. The noise lasts only at the start of a Textured syllable. These are synthesis experiments; listening determines whether they sound more birdlike.

The separate [variable-register demonstration](../../output/birdsong-shapes/04-variable-register-6fe700dda77c.mp3) uses Short chirps. Its three bouts start at 0:00, 0:08 and 0:16, choosing registers of 8, 11 and 9 semitones respectively.

To reproduce these previews with distinct playback filenames:

```sh
python3 tools/audio/render.py --variations --output output/birdsong-shapes
```

## Reproduce the previews

From the repository root, run:

```sh
python3 tools/audio/render.py
```

The renderer needs Python 3 and a C++ compiler. It also creates MP3 files and the combined comparison when `ffmpeg` is installed. Generated WAV files are ignored by Git. The new voices use seed `20261122`, so the previews are reproducible.

`BirdVoice.h` supplies the synthesis code for both the firmware and the renderer. It uses integer arithmetic, a flash-resident sine table and a fixed array of syllables. Audio runs at 16384 Hz. Control runs at 256 Hz, with pitch and volume ramps between control updates.

The reference executes the existing sketch with desktop replacements for the Arduino clock, random generator and Mozzi oscillator. Its random sequence differs from an Uno's. It represents the existing synthesis method, rather than a recording of a particular physical Ara.

The MP3 files use the same output scale, without loudness normalization, reverb or compression effects. They are resampled to 44100 Hz for encoding. They do not reproduce the PWM output circuit, speaker or enclosure. The physical result still needs a listening test.

## Validation

The current Uno firmware uses 9274 bytes of flash and 926 bytes of static RAM with the bundled Mozzi 2.0.2 and Arduino AVR core 1.8.6. This revision has not been uploaded. Sustained runtime timing remains unmeasured. Earlier verified uploads are recorded in the dated sections above.

The desktop renderer runs with address and undefined-behavior sanitizers. It checks output range, phrase completion, return to silence and restart across 100 seeds for all nine voices, with both fixed and variable registers. These 1800 phrases exercise every register in the default range and check that the register remains fixed during each song. Older calls must finish within six seconds; the longer songs must finish within twelve seconds. The oscillator is limited to 7000 Hz, below the 8192 Hz Nyquist limit. These checks do not assess how natural the birdsong sounds.

The current sequence of 200 random songs at 350% gain produced 167 melody/trill songs and 15 short calls, and exercised all 48 melodic contour variations. Forty-eight first phrases contained a held note, with 15 opening on it. Held notes are checked against the chosen repeat or new melody behavior, including both occurrences in an exact repeat. Voice and register must remain fixed through both phrases. New contours must avoid the last three; songs must finish and return to silence. The measured output peak was 213 against a bias of 244. The 7000 Hz limit was reached on 592 of 192516 active control updates.

A separate check of 5000 initial song choices selected registers 13, 15 and 17 respectively 2210, 2688 and 102 times, approximately 44%, 54% and 2%. Across 5000 song choices, 3020 selected exact repeats, approximately 60%. An additional 1800 melody-pair comparisons check that repeats preserve pitch, level, rhythm and held gestures at every control update, while new second melodies differ. Both phrases must finish within the existing duration limit. Further comparisons at gains of 500 and 65535 check that excessive requests clamp without wrapping.

A one-off comparison against the preceding engine checked 589824 samples across all six voices at each of 100%, 125%, 150%, 175% and 200% gain. Every sample matched. Other percentage settings use a rounded fixed-point multiplier and can differ slightly. `tools/audio/check_avr_audio.py --compiler /path/to/avr-g++` compiles a sample wrapper for ATmega328P and rejects division helper calls in its assembly. This guards against the expensive arithmetic returning; it does not measure actual buffer underruns.

To build with Arduino CLI:

```sh
arduino-cli compile --fqbn arduino:avr:uno --libraries sketches/libraries sketches/ara_birdsong
```
