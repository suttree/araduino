#!/usr/bin/env python3
"""Render the shared bird voice and unchanged legacy sketch with desktop adapters."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, default=ROOT / 'output/birdsong')
args = parser.parse_args()
output = args.output.resolve()
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
    subprocess.run([str(executable), str(output)], check=True)

if shutil.which('ffmpeg'):
    for source in sorted(output.glob('*.wav')):
        subprocess.run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y',
                        '-i', str(source), '-ar', '44100', '-codec:a', 'libmp3lame',
                        '-b:a', '128k', str(source.with_suffix('.mp3'))], check=True)
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
print(output)
