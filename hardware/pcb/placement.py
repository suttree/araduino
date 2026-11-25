#!/usr/bin/env python3
"""Apply part-specific supplier origin/rotation conventions to a KiCad CPL."""
import argparse
import csv
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent


def corrected(row, rules):
    result = dict(row)
    rule = rules.get(row['Designator'], {})
    result['Mid X'] = f"{float(row['Mid X']) + rule.get('offset_x_mm', 0):.6f}"
    result['Mid Y'] = f"{float(row['Mid Y']) + rule.get('offset_y_mm', 0):.6f}"
    result['Rotation'] = f"{rule.get('rotation', float(row['Rotation'])) % 360:.6f}"
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path, help='Uncorrected JLC-column CPL')
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    assert args.input.resolve() != args.output.resolve(), 'Retain the uncorrected source CPL'
    rules = json.loads((HERE / 'factory-placement.json').read_text())['parts']
    with args.input.open(newline='') as f:
        rows = list(csv.DictReader(f))
    with args.output.open('w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=rows[0].keys())
        writer.writeheader()
        writer.writerows(corrected(row, rules) for row in rows)
    print('Applied factory conventions to', len(rows), 'placements')


if __name__ == '__main__':
    main()
