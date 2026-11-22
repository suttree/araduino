#!/usr/bin/env python3
"""Check that the Uno audio sample function does not call division helpers."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--compiler', type=Path, required=True, help='Path to avr-g++')
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix='ara-avr-audio-') as directory:
    temporary = Path(directory)
    source = temporary / 'audio.cpp'
    source.write_text(
        '#include "BirdVoice.h"\n'
        'extern "C" __attribute__((noinline)) int8_t audioSample(ara::BirdVoice *voice) {\n'
        '  return voice->audio();\n}\n')
    assembly = temporary / 'audio.s'
    subprocess.run([str(args.compiler), '-std=gnu++11', '-Os', '-mmcu=atmega328p',
                    '-I', str(ROOT / 'sketches/ara_birdsong'), '-S', str(source),
                    '-o', str(assembly)], check=True)
    match = re.search(r'^audioSample:\n(.*?)^\s*\.size\s+audioSample',
                      assembly.read_text(), re.MULTILINE | re.DOTALL)
    if not match:
        raise RuntimeError('Could not locate the compiled audio sample function')
    calls = re.findall(r'\b(?:r?call|jmp)\s+([^\s;]+)', match.group(1))
    if any('div' in call for call in calls):
        raise RuntimeError(f'Audio sample function calls division helpers: {calls}')
    print(f'AVR audio helper calls: {calls}')
    print('No division helpers in the audio sample function. Runtime timing still needs testing.')
