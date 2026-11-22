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

All new voices generate a motif, repeat it with a small pitch offset, then stop. They use frequencies measured in hertz rather than notes chosen from scales. Pitch and volume change throughout each syllable. The waveform is still a sine wave, with no recordings, effects or added background audio.

The new voices now sing approximately eleven semitones higher than the first experiment, three semitones above the previous revision. Their pitch contours are multiplied by 1.888, with timing and volume preserved. The [register comparison](../../output/birdsong/soft-register-comparison.mp3) plays the original soft voice first, then the higher version at 0:26.5. Both use the same seed and phrases. The [original register](../../output/birdsong/04-soft-original-register.mp3) is also available separately. Passing `1000` as the second `BirdVoice` constructor argument selects that original register.

The [earlier short pitch comparison](../../output/birdsong/ara-register-steps-original-five-eight-5a592528fa73.mp3) plays the same phrase in its original register at 0:00, five semitones higher at 0:05, and eight semitones higher at 0:10. The [eight-semitone soft voice with a distinct filename](../../output/birdsong/ara-soft-plus-eight-semitones-b014f8cdde3f.mp3) remains available. A stale player cache is a possible explanation for hearing the same pitch, but has not been confirmed.

Run `python3 tools/audio/compare_registers.py` to recreate these files from the three saved Git revisions. This requires Git and ffmpeg. The script checks pitch ratios by counting waveform cycles with a threshold that excludes low-level encoding noise. The measured ratios are 1.3352 and 1.5862 relative to the original. It records results in `output/birdsong/register-verification.json` and gives each published audio file a filename containing its content hash.

After rendering the current voice, run `python3 tools/audio/compare_registers.py --current` for the [eight-versus-eleven comparison](../../output/birdsong/ara-register-steps-eight-eleven-a0ee560fcd89.mp3). The previous register plays at 0:00 and the new register at 0:05. The [new soft voice](../../output/birdsong/ara-soft-plus-eleven-semitones-0c4e28d480c3.mp3), [garden calls](../../output/birdsong/ara-garden-plus-eleven-semitones-84c231880d9a.mp3) and [lively trills](../../output/birdsong/ara-lively-plus-eleven-semitones-d11c2885fdfb.mp3) have distinct filenames. The measured pitch ratio is 1.8858 relative to the original, approximately 19% higher than the eight-semitone version.

The soft voice is the firmware default. Change `ara::BirdVoice::Soft` to `Garden` or `Lively` in the sketch to try the others. The firmware retains the original interval between attempts and its chance of remaining silent.

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

The Uno build uses 5528 bytes of flash and 839 bytes of static RAM with the bundled Mozzi 2.0.2 and Arduino AVR core 1.8.6. This confirms compilation and memory fit. Runtime timing and physical playback remain untested. Nothing was uploaded during this experiment.

The desktop renderer runs with address and undefined-behavior sanitizers. It checks output range, phrase completion, return to silence and restart across 100 seeds for each new voice. These checks do not assess how natural the birdsong sounds.

To build with Arduino CLI:

```sh
arduino-cli compile --fqbn arduino:avr:uno --libraries sketches/libraries sketches/ara_birdsong
```
