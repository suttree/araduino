#!/usr/bin/env python3
"""Compare saved soft-voice revisions and give playback files unique names."""
import array
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / 'output/birdsong'
REVISIONS = [('original', '6ef77a7', 1.0), ('plus-five', '5d67622', 1.335),
             ('plus-eight', '3302552', 1.587)]


def run(arguments, **kwargs):
    return subprocess.run(arguments, check=True, capture_output=True, **kwargs).stdout


def cycle_count(source):
    pcm = run(['ffmpeg', '-v', 'error', '-i', str(source), '-t', '4',
               '-f', 's16le', '-ac', '1', '-ar', '44100', 'pipe:1'])
    samples = array.array('h', pcm)
    if sys.byteorder != 'little':
        samples.byteswap()
    # Hysteresis ignores low-level encoding noise around silence.
    armed = False
    cycles = 0
    for sample in samples:
        if sample < -1500:
            armed = True
        elif sample > 1500 and armed:
            cycles += 1
            armed = False
    return cycles


def publish(source, name):
    digest = hashlib.sha256(source.read_bytes()).hexdigest()[:12]
    destination = OUTPUT / f'{name}-{digest}.mp3'
    shutil.copyfile(source, destination)
    return destination


with tempfile.TemporaryDirectory(prefix='ara-register-check-') as directory:
    temporary = Path(directory)
    sources = []
    report = []
    for label, revision, expected in REVISIONS:
        source = temporary / f'{label}.mp3'
        source.write_bytes(run(['git', '-C', str(ROOT), 'show',
                                f'{revision}:output/birdsong/01-soft-whistles.mp3']))
        sources.append(source)
        cycles = cycle_count(source)
        baseline = report[0]['cycles'] if report else cycles
        ratio = cycles / baseline
        if abs(ratio - expected) > 0.01:
            raise RuntimeError(f'{label}: unexpected pitch ratio {ratio}')
        report.append({'label': label, 'revision': revision, 'cycles': cycles,
                       'ratio': round(ratio, 4)})
    command = ['ffmpeg', '-v', 'error', '-y']
    for source in sources:
        command += ['-i', str(source)]
    filters = ';'.join(
        f'[{i}:a]atrim=end=4,asetpts=PTS-STARTPTS,apad=pad_dur=1[a{i}]'
        for i in range(3))
    filters += ';[a0][a1][a2]concat=n=3:v=0:a=1[out]'
    comparison = temporary / 'comparison.mp3'
    run(command + ['-filter_complex', filters, '-map', '[out]', '-ar', '44100',
                   '-codec:a', 'libmp3lame', '-b:a', '128k', str(comparison)])
    print(publish(comparison, 'ara-register-steps-original-five-eight'))
    print(publish(sources[-1], 'ara-soft-plus-eight-semitones'))
    (OUTPUT / 'register-verification.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))
