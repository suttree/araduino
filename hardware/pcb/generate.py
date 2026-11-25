#!/usr/bin/env python3
"""Create the Ara prototype PCB with KiCad's pcbnew Python module.

Run with KiCad's Python. Footprints are frozen in this directory. Routing is
imported separately so rerunning this script never silently loses a finished PCB.
"""
import argparse
import importlib.util
import json
from pathlib import Path

import pcbnew as k
import wx

HERE = Path(__file__).resolve().parent
SCHEMATIC = HERE.parent / 'schematic'
app = wx.App(False)

# Coordinates in millimetres. All fitted components are on the front.
PLACEMENTS = {
    'J1': (52.5, 70, 270), 'R1': (59.5, 77.5, 270), 'R2': (59.5, 62, 90),
    'F1': (60, 57, 0), 'C1': (63, 54, 0), 'U4': (67, 57, 0),
    'C2': (71, 56.5, 270), 'C3': (73.5, 54, 0), 'U5': (60.3, 70, 0),
    'U1': (80, 70, 0), 'C4': (73, 69.6, 180), 'C5': (73, 71.8, 180),
    'FB1': (88, 72, 180), 'C6': (87, 74.5, 0), 'C7': (87, 69, 0),
    'Y1': (73, 75, 270), 'R3': (78, 62, 270),
    'U2': (66.5, 68.5, 0), 'C8': (69.5, 61.5, 0), 'C9': (61, 66, 90),
    'R4': (72, 64.5, 0), 'R5': (72, 66.5, 0),
    'C10': (70, 60, 0), 'D1': (75, 59, 90),
    'R6': (80.5, 78, 270), 'R7': (84.5, 79.5, 0),
    'C11': (80.5, 82, 270), 'C12': (87, 82, 270),
    'R8': (89.5, 79.5, 0), 'R9': (93, 82, 270), 'C13': (93, 79.5, 0),
    'J2': (83, 56, 0), 'SW1': (95, 57, 0),
    'TP1': (77, 55, 0), 'TP2': (79, 58, 0),
    'TP3': (80.5, 86, 0), 'TP4': (90, 66, 0),
    'U3': (100, 78, 0), 'FB2': (105, 84, 0), 'FB3': (106, 73.5, 0),
    'J3': (110, 78, 0), 'C14': (96, 83, 0), 'C15': (104.7, 78, 90),
    'C16': (101.5, 71.5, 0), 'R10': (94, 72, 0),
    'C17': (94, 75, 270), 'R11': (90.5, 69.5, 0),
    'C18': (106, 87, 270), 'C19': (109, 73, 90),
}


def pt(x, y):
    return k.VECTOR2I(k.FromMM(x), k.FromMM(y))


def load_footprint(board, lib, name):
    fp = k.FootprintLoad(str(HERE / 'footprints' / (lib + '.pretty')), name)
    if fp is None:
        raise RuntimeError('Missing footprint ' + lib + ':' + name)
    board.Add(fp)
    return fp


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=HERE / 'ara-unrouted.kicad_pcb')
    args = parser.parse_args()
    parts = json.loads((SCHEMATIC / 'circuit.json').read_text())
    spec = importlib.util.spec_from_file_location('ara_circuit', SCHEMATIC / 'generate.py')
    circuit = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(circuit)
    board = k.BOARD()
    board.SetFileName(str(args.output))
    board.SetCopperLayerCount(2)
    settings = board.GetDesignSettings()
    settings.m_MinClearance = k.FromMM(0.127)
    settings.m_TrackMinWidth = k.FromMM(0.15)
    settings.m_CopperEdgeClearance = k.FromMM(0.3)
    settings.m_HoleClearance = k.FromMM(0.25)
    settings.m_ViasMinAnnularWidth = k.FromMM(0.15)
    settings.SetAuxOrigin(pt(50, 95))
    default = board.GetAllNetClasses()['Default']
    default.SetClearance(k.FromMM(0.15))
    default.SetTrackWidth(k.FromMM(0.25))
    default.SetViaDiameter(k.FromMM(0.65))
    default.SetViaDrill(k.FromMM(0.3))
    names = sorted({pin['net'] for p in parts for pin in p['pins'] if pin['net']})
    nets = {}
    for name in names:
        nets[name] = k.NETINFO_ITEM(board, name)
        board.Add(nets[name])
    for p in parts:
        if p['kind'] == 'flag':
            continue
        lib, name = p['footprint'].split(':')
        fp = load_footprint(board, lib, name)
        fp.SetFPIDAsString(p['footprint'])
        fp.SetReference(p['ref'])
        fp.SetValue(p['value'])
        for key, value in [('MPN', p['mpn']), ('LCSC', p['lcsc']), ('DesignNote', p['note'])]:
            fp.SetField(key, value)
            fp.GetField(key).SetVisible(False)
            fp.GetField(key).SetLayer(k.F_Fab)
        fp.SetPath(k.KIID_PATH('/' + circuit.ROOT + '/' + circuit.uid(p['ref'])))
        fp.SetSheetfile('ara.kicad_sch')
        fp.SetSheetname('')
        x, y, angle = PLACEMENTS[p['ref']]
        fp.SetPosition(pt(x, y))
        fp.SetOrientationDegrees(angle)
        fp.Value().SetVisible(False)
        fp.Reference().SetTextSize(pt(0.8, 0.8))
        fp.Reference().SetTextThickness(k.FromMM(0.12))
        fp.Reference().SetTextAngle(k.EDA_ANGLE(0, k.DEGREES_T))
        # Board review uses the assembly drawing for dense reference labels.
        # Keep component references on fabrication rather than over pads.
        fp.Reference().SetLayer(k.F_Fab)
        expected = {pin['number']: pin['net'] for pin in p['pins']}
        actual = {pad.GetNumber() for pad in fp.Pads() if pad.GetNumber()}
        assert actual == set(expected), (p['ref'], actual, set(expected))
        for pad in fp.Pads():
            name = pad.GetNumber()
            if not name:
                continue
            net = expected[name]
            if net:
                pad.SetNet(nets[net])
        if p['ref'].startswith('TP'):
            fp.SetAttributes(fp.GetAttributes() | k.FP_EXCLUDE_FROM_BOM | k.FP_EXCLUDE_FROM_POS_FILES)
    for index, (x, y) in enumerate([(53.5, 53.5), (111.5, 53.5), (53.5, 91.5), (111.5, 91.5)], 1):
        fp = load_footprint(board, 'MountingHole', 'MountingHole_3.2mm_M3')
        fp.SetReference('H' + str(index))
        fp.SetBoardOnly(True)
        fp.SetPosition(pt(x, y))
        fp.Reference().SetVisible(False)
        fp.Value().SetVisible(False)
        fp.SetAttributes(fp.GetAttributes() | k.FP_EXCLUDE_FROM_BOM | k.FP_EXCLUDE_FROM_POS_FILES)
    for a, b in [((50, 50), (115, 50)), ((115, 50), (115, 95)),
                 ((115, 95), (50, 95)), ((50, 95), (50, 50))]:
        line = k.PCB_SHAPE()
        line.SetShape(k.SHAPE_T_SEGMENT)
        line.SetStart(pt(*a))
        line.SetEnd(pt(*b))
        line.SetLayer(k.Edge_Cuts)
        line.SetWidth(k.FromMM(0.05))
        board.Add(line)
    for text, x, y, size in [('ARA  REV A', 83, 91.5, 1.4),
                              ('USB-C 5V', 59.5, 81, 0.9),
                              ('SPEAKER', 108, 89, 0.9),
                              ('ISP  1', 84, 52.5, 0.9),
                              ('RESET', 95, 52.5, 0.9),
                              ('PROTOTYPE', 83, 94, 0.8)]:
        t = k.PCB_TEXT(board)
        t.SetText(text)
        t.SetPosition(pt(x, y))
        t.SetTextSize(pt(size, size))
        t.SetTextThickness(k.FromMM(0.15))
        t.SetLayer(k.F_SilkS)
        board.Add(t)
    # Remove connector outline strokes outside the board from printed silk.
    for fp in board.GetFootprints():
        for graphic in fp.GraphicalItems():
            if graphic.GetLayer() != k.F_SilkS:
                continue
            box = graphic.GetBoundingBox()
            if box.GetLeft() < k.FromMM(50.2) or box.GetRight() > k.FromMM(114.8):
                graphic.SetLayer(k.F_Fab)
    project = {
        'meta': {'filename': args.output.with_suffix('.kicad_pro').name, 'version': 1},
        'board': {'design_settings': {'rules': {
            'min_clearance': 0.127, 'min_track_width': 0.15,
            'min_copper_edge_clearance': 0.3, 'min_hole_clearance': 0.25,
            'min_via_annular_width': 0.15, 'min_via_diameter': 0.6,
            'min_through_hole_diameter': 0.3}}},
        'net_settings': {'classes': [{'name': 'Default', 'clearance': 0.15,
            'track_width': 0.25, 'via_diameter': 0.65, 'via_drill': 0.3,
            'diff_pair_width': 0.2, 'diff_pair_gap': 0.15, 'bus_width': 12,
            'line_style': 0, 'microvia_diameter': 0.3, 'microvia_drill': 0.1,
            'pcb_color': 'rgba(0, 0, 0, 0.000)', 'schematic_color': 'rgba(0, 0, 0, 0.000)',
            'wire_width': 6}], 'meta': {'version': 3}, 'netclass_assignments': {},
            'netclass_patterns': []}}
    args.output.with_suffix('.kicad_pro').write_text(json.dumps(project, indent=2)+'\n')
    board.BuildListOfNets()
    board.GetConnectivity().Build(board)
    k.SaveBoard(str(args.output), board)
    dsn = args.output.with_suffix('.dsn')
    assert k.ExportSpecctraDSN(board, str(dsn)), 'DSN export failed'
    print('Saved', args.output, 'and', dsn)


if __name__ == '__main__':
    main()
