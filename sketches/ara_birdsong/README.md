# Birdsong voice experiment

This sketch experiments with shaped whistles and repeated calls on the existing Uno hardware. The working sketch at `../araduino/araduino.ino` remains separate.

## Listen

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

The firmware chooses a register for each song. These constants near the top of `ara_birdsong.ino` set the inclusive range in semitones above the original experiment:

```cpp
constexpr uint8_t MIN_REGISTER_SEMITONES = 8;
constexpr uint8_t MAX_REGISTER_SEMITONES = 11;
```

The default range includes 8, 9, 10 and 11 semitones. Set both limits to 11 for the highest register auditioned so far. The supported range is 0 through 12. Values outside that range are clamped, and reversed limits are swapped. The chosen register remains fixed through the song and its repeat.

The firmware chooses a voice at random for each song, with equal probability for Soft, Garden, Lively, Chirps, Liquid and Textured. Lively includes the trills. The voice remains fixed through the song and its repeat. Consecutive songs can choose the same voice. The firmware retains the original interval between attempts and its chance of remaining silent. To audition a fixed voice, replace the two `voice.triggerRandom()` calls with a call such as `voice.trigger(ara::BirdVoice::Chirps)`.

`OUTPUT_GAIN_PERCENT` sets the firmware volume. It is now 150, a 50% increase in digital amplitude over the previews, or approximately 3.5 dB. The voice limits this setting to 200 to preserve output headroom. This gain change does not alter pitch or timing. Existing desktop previews remain at the original 100% level.

`NOISY_ONSETS` is currently `false` in the firmware. The owner reported crackling only during songs after increasing the volume. This setting disables the experimental noise at the start of Textured syllables while preserving the second tone, all six voice choices and 150% gain. It tests a possible cause; listening must establish whether it removes the reported crackle. Existing audition recordings retain their original noise setting.

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

The Uno firmware uses 6698 bytes of flash and 853 bytes of static RAM with the bundled Mozzi 2.0.2 and Arduino AVR core 1.8.6. Earlier builds with a fixed Soft or Textured profile also passed. The owner confirmed physical playback and the increased volume at 150% gain, then reported crackle during songs. The version with noisy onsets disabled was uploaded on 22 November 2026, and avrdude verified all 6698 bytes by reading back the flash. Listening with noisy onsets disabled and sustained runtime timing remain unconfirmed.

The desktop renderer runs with address and undefined-behavior sanitizers. It checks output range, phrase completion, return to silence and restart across 100 seeds for all six voices, with both fixed and variable registers. The 1200 phrases exercise every register in the default range and check that the register remains fixed during each song. The oscillator is limited to 7000 Hz, below the 8192 Hz Nyquist limit. These checks do not assess how natural the birdsong sounds.

An additional sequence of 100 random songs at 150% gain checks that all six profiles are selected, including Lively, that voice and register stay fixed during a song, that output stays within its expected headroom, and that every song finishes within the expected duration and returns to silence.

To build with Arduino CLI:

```sh
arduino-cli compile --fqbn arduino:avr:uno --libraries sketches/libraries sketches/ara_birdsong
```
