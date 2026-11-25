#!/usr/bin/env python3
"""Inspect fabrication details that native ERC/DRC do not establish."""
import collections
import json
import math
from pathlib import Path

import pcbnew as k
import wx

HERE = Path(__file__).resolve().parent
app = wx.App(False)
board = k.LoadBoard(str(HERE.parent / 'schematic' / 'ara.kicad_pcb'))
fitted = {ref for group in json.loads((HERE / 'parts.json').read_text())['groups']
          for ref in group['references']}
pads = [(fp.GetReference(), pad) for fp in board.GetFootprints() for pad in fp.Pads()
        if fp.GetReference() in fitted and pad.GetAttribute() == k.PAD_ATTRIB_SMD
        and pad.GetLayerSet().Contains(k.F_Paste)]
overlaps = []
lengths = collections.defaultdict(float)
vias = collections.Counter()
for track in board.GetTracks():
    if not isinstance(track, k.PCB_VIA):
        lengths[track.GetNetname()] += k.ToMM(track.GetLength())
        continue
    vias[track.GetNetname()] += 1
    for ref, pad in pads:
        dx = k.ToMM(track.GetPosition().x - pad.GetPosition().x)
        dy = k.ToMM(track.GetPosition().y - pad.GetPosition().y)
        a = math.radians(pad.GetOrientationDegrees())
        x = abs(dx * math.cos(a) - dy * math.sin(a))
        y = abs(dx * math.sin(a) + dy * math.cos(a))
        sx, sy = k.ToMM(pad.GetSize().x) / 2, k.ToMM(pad.GetSize().y) / 2
        distance = math.hypot(max(0, x - sx), max(0, y - sy))
        if distance < k.ToMM(track.GetDrillValue()) / 2 + 0.05:
            overlaps.append({'net': track.GetNetname(), 'reference': ref, 'pad': pad.GetNumber()})
assert not overlaps, overlaps
for net in ['/XTAL1', '/XTAL2']:
    assert vias[net] == 0 and lengths[net] < 6, (net, lengths[net], vias[net])
for fp in board.GetFootprints():
    if fp.GetReference() not in fitted:
        continue
    assert fp.GetLayer() == k.F_Cu, fp.GetReference()
    for pad in fp.Pads():
        for x, y in [(53.5, 53.5), (111.5, 53.5), (53.5, 91.5), (111.5, 91.5)]:
            # Circumscribed pad radius gives a conservative screw-head check.
            radius = math.hypot(k.ToMM(pad.GetSize().x), k.ToMM(pad.GetSize().y)) / 2
            separation = math.hypot(k.ToMM(pad.GetPosition().x) - x,
                                    k.ToMM(pad.GetPosition().y) - y)
            assert separation - radius > 3.2, (fp.GetReference(), pad.GetNumber())
report = {
    'status': 'PASS: fabrication geometry inspection, not physical testing',
    'fitted_components': len(fitted),
    'vias_overlapping_fitted_smd_paste': overlaps,
    'all_fitted_components_on_top': True,
    'fitted_pads_outside_screw_head_areas': True,
    'critical_net_geometry': {name: {'total_trace_length_mm': round(lengths[name], 3), 'vias': vias[name]}
        for name in ['/XTAL1', '/XTAL2', '/USB_DP', '/USB_DM', '/AMP_INP', '/AMP_INM']},
    'limitations': ['No electrical simulation or measurement', 'Factory part rotations not verified',
                    'Trace lengths are geometry totals, not a USB signal-integrity test'],
}
out = HERE.parent / 'manufacturing' / 'rev-a' / 'checks' / 'fabrication.json'
out.write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
