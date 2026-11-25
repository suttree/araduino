#!/usr/bin/env python3
"""Import checked routing, add ground fills and save the editable project board."""
import argparse
import json
import math
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

import pcbnew as k
import wx

HERE = Path(__file__).resolve().parent
SCHEMATIC = HERE.parent / 'schematic'
app = wx.App(False)


def pt(x, y):
    return k.VECTOR2I(k.FromMM(x), k.FromMM(y))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('session', type=Path)
    p.add_argument('--input', type=Path, default=HERE / 'ara-unrouted.kicad_pcb')
    p.add_argument('--netlist', type=Path, required=True, help='Native KiCad XML netlist')
    args = p.parse_args()
    board = k.LoadBoard(str(args.input))
    assert k.ImportSpecctraSES(board, str(args.session.resolve())), 'Routing import failed'
    # KiCad prefixes local sheet labels with '/'. Preserve its exact names,
    # including each deliberately unconnected pin, for schematic parity checks.
    native = ET.parse(args.netlist)
    pad_nets = {}
    for node in native.findall('./nets/net'):
        name = node.get('name')
        for pin in node.findall('node'):
            pad_nets[pin.get('ref'), pin.get('pin')] = name
    for net in list(board.GetNetsByNetcode().values()):
        if net.GetNetname():
            net.SetNetname('/' + net.GetNetname())
    for fp in board.GetFootprints():
        for pad in fp.Pads():
            name = pad_nets.get((fp.GetReference(), pad.GetNumber()))
            if name is None:
                continue
            net = board.FindNet(name)
            if net is None:
                net = k.NETINFO_ITEM(board, name)
                board.Add(net)
            pad.SetNet(net)
    # Leave a 6.4 mm diameter copper-free area beneath each M3 screw head.
    for x, y in [(53.5, 53.5), (111.5, 53.5), (53.5, 91.5), (111.5, 91.5)]:
        area = k.ZONE(board)
        area.SetIsRuleArea(True)
        layers = k.LSET()
        layers.AddLayer(k.F_Cu)
        layers.AddLayer(k.B_Cu)
        area.SetLayerSet(layers)
        area.SetDoNotAllowTracks(True)
        area.SetDoNotAllowVias(True)
        # The mounting hole itself is an NPTH pad. Track/via/fill keepouts
        # remove copper without flagging that mechanical pad as a violation.
        area.SetDoNotAllowPads(False)
        area.SetDoNotAllowZoneFills(True)
        area.SetDoNotAllowFootprints(False)
        poly = area.Outline()
        poly.NewOutline()
        for index in range(64):
            a = index * 2 * math.pi / 64
            poly.Append(k.FromMM(x + 3.2 * math.cos(a)), k.FromMM(y + 3.2 * math.sin(a)))
        board.Add(area)
    gnd = board.FindNet('/GND')
    for layer in (k.F_Cu, k.B_Cu):
        zone = k.ZONE(board)
        zone.SetLayer(layer)
        zone.SetNet(gnd)
        zone.SetLocalClearance(k.FromMM(0.2))
        zone.SetThermalReliefGap(k.FromMM(0.25))
        zone.SetThermalReliefSpokeWidth(k.FromMM(0.3))
        zone.SetMinThickness(k.FromMM(0.2))
        outline = zone.Outline()
        outline.NewOutline()
        for x, y in [(50.4, 50.4), (114.6, 50.4), (114.6, 94.6), (50.4, 94.6)]:
            outline.Append(k.FromMM(x), k.FromMM(y))
        board.Add(zone)
    board.BuildListOfNets()
    board.GetConnectivity().Build(board)
    k.ZONE_FILLER(board).Fill(board.Zones())
    output = SCHEMATIC / 'ara.kicad_pcb'
    board.SetFileName(str(output))
    k.SaveBoard(str(output), board)
    project = json.loads(args.input.with_suffix('.kicad_pro').read_text())
    project['meta']['filename'] = 'ara.kicad_pro'
    (SCHEMATIC / 'ara.kicad_pro').write_text(json.dumps(project, indent=2) + '\n')
    shutil.copyfile(args.session, HERE / 'ara-routed.ses')
    print('Saved', output)


if __name__ == '__main__':
    main()
