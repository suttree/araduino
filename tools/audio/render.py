#!/usr/bin/env python3
"""Render the shared bird voice and unchanged legacy sketch with desktop adapters."""
import argparse
import hashlib
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path)
parser.add_argument('--variations', action='store_true', help='Audition alternative syllable shapes and timbres')
args = parser.parse_args()
output = (args.output or ROOT / ('output/birdsong-shapes' if args.variations else 'output/birdsong')).resolve()
output.mkdir(parents=True, exist_ok=True)
with tempfile.TemporaryDirectory(prefix='ara-audio-') as temporary:
    build = Path(temporary)
    # The reference sketch needs Arduino/Mozzi names, not their hardware drivers.
    for name in ('Arduino.h', 'MozziGuts.h', 'Oscil.h'):
        (build / name).write_text('#pragma once\n#define AUDIO_RATE 16384\n')
    (build / 'mozzi_pgmspace.h').write_text(
        '#pragma once\n#define CONSTTABLE_STORAGE(...) const __VA_ARGS__\n'
        '#define FLASH_OR_RAM_READ(ptr) (*(ptr))\n')
    executable = build / 'render'
    subprocess.run(['c++', '-std=c++11', '-O2', '-Wall', '-Wextra',
                    '-fsanitize=undefined,address', '-I', str(build),
                    '-I', str(ROOT / 'sketches/libraries/Mozzi'),
                    str(ROOT / 'tools/audio/render.cpp'), '-o', str(executable)], check=True)
    subprocess.run([str(executable), str(output)] + (['--variations'] if args.variations else []), check=True)

if shutil.which('ffmpeg'):
    for source in sorted(output.glob('*.wav')):
        subprocess.run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y',
                        '-i', str(source), '-ar', '44100', '-codec:a', 'libmp3lame',
                        '-b:a', '128k', str(source.with_suffix('.mp3'))], check=True)
    if args.variations:
        # Match the first four registers and seeds, then audition each first bout.
        sources = sorted(output.glob('0[0-3]-*.wav'))
        command = ['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y']
        for source in sources:
            command += ['-i', str(source)]
        filters = ';'.join(
            f'[{i}:a]atrim=end=4,asetpts=PTS-STARTPTS,apad=pad_dur=1[a{i}]'
            for i in range(4))
        filters += ';[a0][a1][a2][a3]concat=n=4:v=0:a=1[out]'
        comparison = output / 'ara-shape-comparison.mp3'
        subprocess.run(command + ['-filter_complex', filters, '-map', '[out]', '-ar', '44100',
                                 '-codec:a', 'libmp3lame', '-b:a', '128k', str(comparison)], check=True)
        recordings = [source.with_suffix('.mp3') for source in sorted(output.glob('0[0-4]-*.wav'))]
        for source in recordings + [comparison]:
            digest = hashlib.sha256(source.read_bytes()).hexdigest()[:12]
            destination = output / f'{source.stem}-{digest}.mp3'
            shutil.copyfile(source, destination)
            print(destination)
        raise SystemExit(0)
    # One comparison file, with half a second of silence between approaches.
    sources = sorted(output.glob('0[0-3]-*.wav'))
    command = ['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y']
    for source in sources:
        command += ['-i', str(source)]
    filters = ';'.join(f'[{i}:a]apad=pad_dur=0.5[a{i}]' for i in range(4))
    filters += ';' + ''.join(f'[a{i}]' for i in range(4)) + 'concat=n=4:v=0:a=1[out]'
    subprocess.run(command + ['-filter_complex', filters, '-map', '[out]', '-ar', '44100',
                             '-codec:a', 'libmp3lame', '-b:a', '128k',
                             str(output / 'ara-voice-comparison.mp3')], check=True)
    subprocess.run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y',
                    '-i', str(output / '04-soft-original-register.wav'),
                    '-i', str(output / '01-soft-whistles.wav'),
                    '-filter_complex',
                    '[0:a]apad=pad_dur=0.5[lower];[lower][1:a]concat=n=2:v=0:a=1[out]',
                    '-map', '[out]', '-ar', '44100', '-codec:a', 'libmp3lame', '-b:a', '128k',
                    str(output / 'soft-register-comparison.mp3')], check=True)
print(output)
