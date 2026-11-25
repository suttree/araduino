#!/usr/bin/env python3
"""Check the saved PCB and export a prototype quotation package with KiCad CLI."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import zipfile
from placement import corrected

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
SCHEMATIC = HERE.parent / 'schematic'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--kicad-cli', default='kicad-cli')
    p.add_argument('--firmware-build', type=Path, required=True)
    args = p.parse_args()
    out = HERE.parent / 'manufacturing' / 'rev-a'
    gerbers = out / 'gerbers'
    reports = out / 'checks'
    firmware = out / 'firmware'
    for directory in (gerbers, reports, firmware):
        directory.mkdir(parents=True, exist_ok=True)
    cli = args.kicad_cli

    def run(*cmd):
        subprocess.run([cli, *map(str, cmd)], check=True)

    sch = SCHEMATIC / 'ara.kicad_sch'
    pcb = SCHEMATIC / 'ara.kicad_pcb'
    run('sch', 'erc', '--format', 'json', '--exit-code-violations', '-o', reports / 'erc.json', sch)
    run('pcb', 'drc', '--format', 'json', '--schematic-parity', '--exit-code-violations',
        '-o', reports / 'drc.json', pcb)
    run('pcb', 'export', 'gerbers', '--layers',
        'F.Cu,B.Cu,F.Mask,B.Mask,F.Silkscreen,B.Silkscreen,F.Paste,B.Paste,Edge.Cuts',
        '--use-drill-file-origin', '-o', str(gerbers) + '/', pcb)
    run('pcb', 'export', 'drill', '--drill-origin', 'plot', '--excellon-separate-th',
        '--generate-report', '--report-path', reports / 'drill.txt', '-o', str(gerbers) + '/', pcb)
    run('pcb', 'export', 'svg', '--mode-single', '--layers', 'F.Cu,F.Silkscreen,Edge.Cuts',
        '--page-size-mode', '2', '-o', out / 'board-top.svg', pcb)
    run('pcb', 'export', 'svg', '--mode-single', '--layers', 'B.Cu,Edge.Cuts',
        '--page-size-mode', '2', '-o', out / 'board-bottom.svg', pcb)
    run('pcb', 'export', 'svg', '--mode-single', '--layers', 'F.Fab,Edge.Cuts',
        '--page-size-mode', '2', '-o', out / 'assembly.svg', pcb)
    run('sch', 'export', 'svg', '-o', str(out / 'schematic') + '/', sch)
    run('pcb', 'export', 'step', '--board-only', '-o', out / 'board-outline.step', pcb)
    parts = json.loads((SCHEMATIC / 'circuit.json').read_text())
    parts = {p['ref']: p for p in parts if p['kind'] != 'flag' and not p['ref'].startswith('TP')}
    groups = json.loads((HERE / 'parts.json').read_text())['groups']
    with (out / 'bom.csv').open('w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['Comment', 'Designator', 'Footprint', 'LCSC Part #'])
        refs = []
        for group in groups:
            selected = [parts[ref] for ref in group['references']]
            refs.extend(group['references'])
            assert len({part['footprint'] for part in selected}) == 1
            assert len({part['value'] for part in selected}) == 1
            w.writerow([selected[0]['value'], ','.join(group['references']),
                        selected[0]['footprint'], group['lcsc']])
        assert sorted(refs) == sorted(parts)
    with tempfile.TemporaryDirectory(prefix='ara-pos-') as directory:
        pos = Path(directory) / 'positions.csv'
        run('pcb', 'export', 'pos', '--format', 'csv', '--units', 'mm',
            '--side', 'front', '--use-drill-file-origin', '--exclude-dnp', '-o', pos, pcb)
        rows = list(csv.DictReader(pos.open()))
        assert sorted(r['Ref'] for r in rows) == sorted(parts), rows
        rules = json.loads((HERE / 'factory-placement.json').read_text())['parts']
        with (out / 'cpl-kicad.csv').open('w', newline='') as f, (out / 'cpl.csv').open('w', newline='') as factory:
            w = csv.writer(f)
            headers = ['Designator', 'Mid X', 'Mid Y', 'Layer', 'Rotation']
            w.writerow(headers)
            target = csv.DictWriter(factory, fieldnames=headers)
            target.writeheader()
            for row in rows:
                assert row['Side'] == 'top', row
                values = [row['Ref'], row['PosX'], row['PosY'], 'Top', row['Rot']]
                w.writerow(values)
                target.writerow(corrected(dict(zip(headers, values)), rules))
    for source, name in [('ara_birdsong.ino.hex', 'ara-birdsong.hex'),
                         ('ara_birdsong.ino.with_bootloader.hex', 'ara-birdsong-with-bootloader.hex')]:
        shutil.copyfile(args.firmware_build / source, firmware / name)
    source_files = ['sketches/ara_birdsong/ara_birdsong.ino', 'sketches/ara_birdsong/BirdVoice.h']
    manifest = {
        'status': 'Prototype quotation package. Supplier CPL corrections applied. Inspect the factory placement preview after every export; physical operation remains unverified.',
        'board_mm': {'width': 65, 'height': 45, 'thickness': 1.6},
        'layers': 2, 'assembly_side': 'Top', 'fitted_components_per_board': len(parts),
        'unique_part_numbers': len(groups), 'native_checks': 'Zero ERC, DRC, unconnected and schematic parity findings.',
        'firmware_source_commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        'firmware_source_sha256': {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in source_files},
        'toolchain': 'Arduino AVR 1.8.6; Mozzi 2.0.2; FixMath 1.0.7; arduino:avr:uno',
        'files_sha256': {},
    }
    with zipfile.ZipFile(out / 'gerbers.zip', 'w', zipfile.ZIP_DEFLATED) as z:
        for path in sorted(gerbers.iterdir()):
            z.write(path, path.name)
    for path in sorted(out.rglob('*')):
        if path.is_file() and path.name != 'manifest.json':
            manifest['files_sha256'][str(path.relative_to(out))] = hashlib.sha256(path.read_bytes()).hexdigest()
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print('Exported', out, 'with', len(parts), 'fitted components')


if __name__ == '__main__':
    main()
