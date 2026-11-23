# Birdsong voice experiment

This sketch experiments with shaped whistles and repeated calls on the existing Uno hardware. The working sketch at `../araduino/araduino.ino` remains separate.

## Less repetition, 23 November

The highest register is now a 10% choice. The lower and middle choices have weights of 40% and 50%. Gain remains at 400%.

The owner reported hearing the same two songs frequently. The earlier song profiles reused one eight-syllable pitch contour, fixed trill positions and three renditions. The generator now varies six contours, four rhythm patterns, phrase length from six to eight syllables, trill positions, sweep directions, note lengths and breaths. Songs use two or three renditions. The answering phrase can exchange the two halves of the motif, and the final rendition can exchange its last two syllables. Cascades vary their pitch step. A song still retains a shared pitch centre and related phrases.

Random voice selection avoids the preceding voice, and successive longer songs avoid the preceding contour. The selection pool still favours songs and trills, but retrying a repeated voice changes the exact profile probabilities. Startup mixes sixteen analogue readings with their timing instead of seeding from one reading. This reduces reliance on a single input value; it does not guarantee different startup sequences.

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

The candidate selection pool gives longer songs 75% of the weight: Song 25%, TrillSong approximately 33%, and Cascade approximately 17%. The remaining 25% is shared equally among the six earlier voices, including Lively's trills. Immediate voice repeats are now retried, so actual profile probabilities depend on the preceding choice. Attempt intervals and the chance of remaining silent are unchanged.

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
constexpr uint8_t LOWER_REGISTER_WEIGHT = 4;
constexpr uint8_t MIDDLE_REGISTER_WEIGHT = 5;
constexpr uint8_t UPPER_REGISTER_WEIGHT = 1;
```

These are the auditioned two, four and six semitones above the preceding +11 upper register. Their probabilities are 40%, 50% and 10% respectively. Ara keeps the chosen register through the whole song and every repeat. `setRegisterChoices()` accepts the three registers and their relative weights; all-zero weights fall back to equal probabilities. The supported pitch table now covers 0 through 17 semitones. Out-of-range register values clamp to 17. The older `setRegisterRange()` method remains available for an equally weighted inclusive range.

Random motifs can exceed the pitch targets in the fixed auditions. The engine retains its 7000 Hz frequency limit; in the latest 200-song check it limited 2196 of 211316 active control updates, approximately 1%. Those parts of the pitch contour flatten at the limit. This is not a claim of unlimited pitch headroom.

The firmware chooses a voice at random using the song weighting above. It retains Soft, Garden, Lively, Chirps, Liquid and Textured alongside Song, TrillSong and Cascade. The voice remains fixed through the song and its repeats. Consecutive songs can choose the same voice. To audition a fixed voice, replace the two `voice.triggerRandom()` calls with a call such as `voice.trigger(ara::BirdVoice::TrillSong)`.

`OUTPUT_GAIN_PERCENT` sets the firmware volume. It is now 400, applied before the output limit, reduced from 500 after the owner confirmed louder playback without audible distortion. The earlier 200% limit was imposed by the experiment's 8-bit sample return type, not by the full Uno PWM output range. The voice now returns a 16-bit sample and accepts gain settings up to 500. Output remains limited to -240 through +240, within Mozzi's default Uno range of -244 through +243. Peaks reaching that limit are clipped, so gain above that point can add distortion. Firmware assertions require the default PWM bias and carrier rate. The early desktop previews remain at 100%; the new song previews use 400%.

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

The Uno firmware uses 8936 bytes of flash and 912 bytes of static RAM with the bundled Mozzi 2.0.2 and Arduino AVR core 1.8.6. The owner confirmed clean playback after the division fix, more realistic birdsong in the enclosure, and louder playback without audible distortion at 500% gain. After trying the weighted higher registers, they requested less frequent use of the highest choice and more song variety. The latest version was uploaded on 23 November 2026, and avrdude verified all 8936 bytes by flash readback. Listening to the new variety remains unconfirmed. Sustained runtime timing remains unmeasured.

The desktop renderer runs with address and undefined-behavior sanitizers. It checks output range, phrase completion, return to silence and restart across 100 seeds for all nine voices, with both fixed and variable registers. These 1800 phrases exercise every register in the default range and check that the register remains fixed during each song. Older calls must finish within six seconds; the longer songs must finish within twelve seconds. The oscillator is limited to 7000 Hz, below the 8192 Hz Nyquist limit. These checks do not assess how natural the birdsong sounds.

An additional sequence of 200 random songs at 400% gain checks that all nine profiles and all six song contours are selected, that longer songs dominate, that consecutive random voices and successive song contours do not repeat, and that voice, motif and register remain fixed during a song. It also checks the 7000 Hz limit, PWM output bounds, completion and return to silence. Register counts were 72, 106 and 22. A separate check of 5000 initial song choices selected registers 13, 15 and 17 respectively 1960, 2550 and 490 times, approximately 39%, 51% and 10%. The output check requires samples above 127 so an accidental return to an 8-bit sample cannot pass. Further comparisons at gains of 500 and 65535 check that excessive requests clamp without wrapping.

A one-off comparison against the preceding engine checked 589824 samples across all six voices at each of 100%, 125%, 150%, 175% and 200% gain. Every sample matched. Other percentage settings use a rounded fixed-point multiplier and can differ slightly. `tools/audio/check_avr_audio.py --compiler /path/to/avr-g++` compiles a sample wrapper for ATmega328P and rejects division helper calls in its assembly. This guards against the expensive arithmetic returning; it does not measure actual buffer underruns.

To build with Arduino CLI:

```sh
arduino-cli compile --fqbn arduino:avr:uno --libraries sketches/libraries sketches/ara_birdsong
```
