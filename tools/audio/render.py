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
group = parser.add_mutually_exclusive_group()
group.add_argument('--variations', action='store_true', help='Audition alternative syllable shapes and timbres')
group.add_argument('--songs', action='store_true', help='Audition longer songs and trills at 400% gain')
group.add_argument('--registers', action='store_true', help='Compare current songs at higher pitches, with fixed phrasing')
group.add_argument('--session', action='store_true', help='Showcase ten melodies at the agreed register settings and 350% gain')
args = parser.parse_args()
default = ('output/birdsong-session-20261124' if args.session else 'output/birdsong-registers' if args.registers else 'output/birdsong-songs' if args.songs
           else 'output/birdsong-shapes' if args.variations else 'output/birdsong')
output = (args.output or ROOT / default).resolve()
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
    mode = (['--session'] if args.session else ['--registers'] if args.registers else ['--songs'] if args.songs
            else ['--variations'] if args.variations else [])
    subprocess.run([str(executable), str(output)] + mode, check=True)

if shutil.which('ffmpeg'):
    for source in sorted(output.glob('*.wav')):
        subprocess.run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y',
                        '-i', str(source), '-ar', '44100', '-codec:a', 'libmp3lame',
                        '-b:a', '128k', str(source.with_suffix('.mp3'))], check=True)
    if args.session:
        source = output / 'ara-melody-session.mp3'
        digest = hashlib.sha256(source.read_bytes()).hexdigest()[:12]
        destination = output / f'ara-melody-session-{digest}.mp3'
        shutil.copyfile(source, destination)
        print(destination)
        raise SystemExit(0)
    if args.variations or args.songs or args.registers:
        # Match the first four registers and seeds, then audition each first bout.
        sources = sorted(output.glob('0[0-3]-*.wav'))
        command = ['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y']
        for source in sources:
            command += ['-i', str(source)]
        durations = [9] * 4 if args.registers else [4, 9, 9, 9] if args.songs else [4] * 4
        padding = 0.5 if args.songs or args.registers else 1
        start = 11 if args.registers else 0
        filters = ';'.join(
            f'[{i}:a]atrim=start={start}:end={start + durations[i]},asetpts=PTS-STARTPTS,apad=pad_dur={padding}[a{i}]'
            for i in range(4))
        filters += ';[a0][a1][a2][a3]concat=n=4:v=0:a=1[out]'
        comparison = output / ('ara-higher-register-comparison.mp3' if args.registers
                               else 'ara-song-comparison.mp3' if args.songs else 'ara-shape-comparison.mp3')
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
