#!/usr/bin/env python3
"""Generate the editable KiCad draft and its review drawings from one circuit model.

No third-party runtime dependencies. Run from any directory with Python 3.
The custom symbols expose every physical pin; there are no hidden power pins.
"""
from __future__ import annotations

import html
import json
from pathlib import Path
import uuid
import xml.etree.ElementTree as ET

HERE = Path(__file__).resolve().parent
NS = uuid.UUID('238aa34c-7389-43e3-8bb3-7ac87a259c14')
ROOT = str(uuid.uuid5(NS, 'ara-rev-a'))
PARTS = []
NOTES = []
PANELS = [
    (12, 32, 280, 160, '1  USB-C and controlled power-up'),
    (305, 32, 275, 160, '2  Arduino-compatible processor'),
    (12, 200, 280, 100, '3  USB programming and automatic reset'),
    (305, 200, 275, 100, '4  PWM filter and quiet-volume attenuation'),
    (12, 310, 280, 92, '5  Recovery programming and test points'),
    (305, 310, 275, 92, '6  Analogue amplifier and speaker connector'),
]

def uid(name):
    return str(uuid.uuid5(NS, name))

def q(value):
    return json.dumps(str(value), ensure_ascii=False)

def snap(value):
    return round(round(value / 1.27) * 1.27, 4)

def part(ref, value, xy, pins, *, kind='box', footprint='', mpn='', note='', lcsc=''):
    """pins: (number, visible name, electrical type, net or None)."""
    x, y = map(snap, xy)
    if kind=='flag':
        positions=[(0,0,'L')]
        width,height=2.54,2.54
    elif kind in ('R', 'C', 'F', 'FB', 'D', 'SW'):
        positions = [(-5.08, 0, 'L'), (5.08, 0, 'R')]
        width, height = 6.35, 2.54
    else:
        left = (len(pins) + 1) // 2
        right = len(pins) - left
        rows = max(left, right)
        width = 30.48 if len(pins) > 8 else 22.86
        height = (rows + 1) * 2.54
        positions = []
        for side, count in [('L', left), ('R', right)]:
            for row in range(count):
                positions.append(((-1 if side == 'L' else 1) * (width / 2 + 2.54),
                                  (row - (count - 1) / 2) * 2.54, side))
    pp = []
    for (number, name, typ, net), (dx, dy, side) in zip(pins, positions):
        pp.append(dict(number=str(number), name=name, type=typ, net=net,
                       dx=dx, dy=dy, side=side))
    item = dict(ref=ref, value=value, x=x, y=y, kind=kind, width=width,
                height=height, footprint=footprint, mpn=mpn, lcsc=lcsc,
                note=note, pins=pp)
    PARTS.append(item)
    return item

def two(ref, value, xy, a, b, kind='R', note='', mpn='', footprint=''):
    if not footprint:
        footprint = {'R':'Resistor_SMD:R_0603_1608Metric',
                     'C':'Capacitor_SMD:C_0603_1608Metric',
                     'FB':'Inductor_SMD:L_0805_2012Metric',
                     'F':'Fuse:Fuse_1206_3216Metric',
                     'D':'Diode_SMD:D_SOD-123',
                     'SW':'Button_Switch_SMD:SW_SPST_TL3342'}.get(kind, '')
    names = ['K', 'A'] if kind == 'D' else ['1', '2']
    return part(ref, value, xy, [(1,names[0],'passive',a),
                                (2,names[1],'passive',b)], kind=kind,
                footprint=footprint, mpn=mpn, note=note)

def note(x, y, text):
    NOTES.append((x, y, text))

# USB-C pin designators are contact identifiers, not sequential schematic numbers.
part('J1', 'TYPE-C-31-M-12', (70, 83), [
    ('A4','VBUS A4','passive','VBUS'), ('A9','VBUS A9','passive','VBUS'),
    ('B4','VBUS B4','passive','VBUS'), ('B9','VBUS B9','passive','VBUS'),
    ('A5','CC1','passive','CC1'), ('B5','CC2','passive','CC2'),
    ('A6','D+ A6','passive','USB_DP'), ('B6','D+ B6','passive','USB_DP'),
    ('A7','D- A7','passive','USB_DM'), ('B7','D- B7','passive','USB_DM'),
    ('A1','GND A1','passive','GND'), ('A12','GND A12','passive','GND'),
    ('B1','GND B1','passive','GND'), ('B12','GND B12','passive','GND'),
    ('A8','SBU1','passive',None), ('B8','SBU2','passive',None),
    ('SH','Shield','passive','GND')],
    footprint='Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12',
    mpn='TYPE-C-31-M-12', lcsc='C165948',
    note='USB 2.0 device / 5 V sink. Recheck current supplier mechanical drawing before layout release.')
two('R1', '5.1k 1%', (155, 60), 'CC1', 'GND')
two('R2', '5.1k 1%', (155, 80), 'CC2', 'GND')
two('F1', '500mA PTC 24V', (155, 100), 'VBUS', 'USB_5V', 'F', mpn='nSMD050-24V')
two('C1', '1uF 50V X5R', (155, 120), 'USB_5V', 'GND', 'C')
part('U4', 'TPS22918DBVR', (235, 70), [
    (1,'VIN','power_in','USB_5V'), (3,'ON','input','USB_5V'),
    (2,'GND','power_in','GND'), (6,'VOUT','power_out','+5V'),
    (4,'CT','passive','SOFTSTART'), (5,'QOD','passive','+5V')],
    footprint='Package_TO_SOT_SMD:SOT-23-6', mpn='TPS22918DBVR')
two('C2', '2.2nF 50V X7R', (235, 105), 'SOFTSTART', 'GND', 'C')
two('C3', '4.7uF 25V X5R', (235, 125), '+5V', 'GND', 'C',
    footprint='Capacitor_SMD:C_0805_2012Metric')
part('U5', 'USBLC6-2SC6', (70, 145), [
    (1,'I/O1','passive','USB_DP'), (3,'I/O2','passive','USB_DM'),
    (2,'GND','power_in','GND'), (6,'I/O1','passive','USB_DP'),
    (4,'I/O2','passive','USB_DM'), (5,'VBUS','passive','VBUS')],
    footprint='Package_TO_SOT_SMD:SOT-23-6', mpn='USBLC6-2SC6')
note(25, 178, 'Separate 5.1k CC pull-downs. USB-C data and power; no PD negotiation.')
note(25, 184, 'U4 limits turn-on slew before downstream bulk capacitors. Verify inrush on hardware.')

mcu = [
    (4,'VCC','power_in','+5V'), (6,'VCC','power_in','+5V'),
    (18,'AVCC','power_in','AVCC'), (3,'GND','power_in','GND'),
    (5,'GND','power_in','GND'), (21,'GND','power_in','GND'),
    (20,'AREF','passive','AREF'), (7,'PB6 / XTAL1','input','XTAL1'),
    (8,'PB7 / XTAL2','output','XTAL2'), (29,'PC6 / RESET','input','RESET_N'),
    (30,'PD0 / RXD','input','MCU_RX'), (31,'PD1 / TXD','output','MCU_TX'),
    (13,'PB1 / D9 PWM','output','AUDIO_PWM'), (2,'PD4 / D4','bidirectional','AMP_CONTROL'),
    (15,'PB3 / MOSI','bidirectional','ISP_MOSI'), (16,'PB4 / MISO','bidirectional','ISP_MISO'),
    (17,'PB5 / SCK','bidirectional','ISP_SCK'), (23,'PC0 / A0','input','ADC0_SEED'),
    (1,'PD3','bidirectional',None), (9,'PD5','bidirectional',None),
    (10,'PD6','bidirectional',None), (11,'PD7','bidirectional',None),
    (12,'PB0','bidirectional',None), (14,'PB2','bidirectional',None),
    (19,'ADC6','input',None), (22,'ADC7','input',None),
    (24,'PC1','bidirectional',None), (25,'PC2','bidirectional',None),
    (26,'PC3','bidirectional',None), (27,'PC4','bidirectional',None),
    (28,'PC5','bidirectional',None), (32,'PD2','bidirectional',None)]
part('U1', 'ATMEGA328P-AU', (372, 90), mcu,
    footprint='Package_QFP:TQFP-32_7x7mm_P0.8mm', mpn='ATMEGA328P-AU', lcsc='C14877',
    note='32-pin TQFP pinout. Arduino Uno 16 MHz / 5 V compatibility. NRND lifecycle.')
two('C4', '100nF 50V X7R', (510, 56), '+5V', 'GND', 'C', note='Close to U1 pin 4')
two('C5', '100nF 50V X7R', (510, 76), '+5V', 'GND', 'C', note='Close to U1 pin 6')
two('FB1', 'BLM21PG221SZ1D', (510, 96), '+5V', 'AVCC', 'FB', mpn='BLM21PG221SZ1D')
two('C6', '100nF 50V X7R', (510, 116), 'AVCC', 'GND', 'C')
two('C7', '100nF 50V X7R', (510, 136), 'AREF', 'GND', 'C')
part('Y1', 'CSTNE16M0V530000R0', (360, 157), [
    (1,'XTAL1','passive','XTAL1'), (2,'GND','passive','GND'),
    (3,'XTAL2','passive','XTAL2')],
    footprint='Crystal:Resonator_SMD_Murata_CSTxExxV-3Pin_3.0x1.1mm',
    mpn='CSTNE16M0V530000R0', lcsc='C341521', note='16 MHz / 15 pF built-in load capacitors. Murata replacement for CSTCE16M0V53-R0.')
two('R3', '10k 1%', (510, 159), '+5V', 'RESET_N')
note(320, 180, '16 MHz resonator includes load capacitors. AREF is decoupled, not tied to +5V.')
note(320, 186, 'A0 remains externally floating, as on the prototype, for the existing randomSeed call.')

part('U2', 'CH340C', (75, 244), [
    (16,'VCC','power_in','+5V'), (1,'GND','power_in','GND'),
    (4,'V3','passive','CH340_V3'), (5,'UD+','bidirectional','USB_DP'),
    (6,'UD-','bidirectional','USB_DM'), (15,'R232','input','GND'),
    (9,'CTS_N','input','GND'), (7,'NC','no_connect',None),
    (2,'TXD','output','USB_TX'), (3,'RXD','input','USB_RX'),
    (13,'DTR_N','output','DTR_N'), (14,'RTS_N','output',None),
    (12,'DCD_N','input',None), (11,'RI_N','input',None),
    (10,'DSR_N','input',None), (8,'OUT_N','output',None)],
    footprint='Package_SO:SOIC-16_3.9x9.9mm_P1.27mm', mpn='CH340C', lcsc='C84681')
two('C8', '100nF 50V X7R', (170, 220), '+5V', 'GND', 'C')
two('C9', '100nF 50V X7R', (245, 220), 'CH340_V3', 'GND', 'C')
two('R4', '1k 1%', (170, 245), 'USB_TX', 'MCU_RX')
two('R5', '1k 1%', (245, 245), 'MCU_TX', 'USB_RX')
two('C10', '100nF 50V X7R', (170, 270), 'DTR_N', 'RESET_N', 'C')
two('D1', '1N4148W', (245, 270), '+5V', 'RESET_N', 'D', mpn='1N4148W')
note(25, 291, 'DTR falling edge resets U1 through C10. D1 clamps the positive reset pulse.')

two('R6', '1k 1%', (365, 220), 'AUDIO_PWM', 'LP1')
two('R7', '2.2k 1%', (490, 220), 'LP1', 'LP2')
two('C11', '22nF 50V X7R', (365, 243), 'LP1', 'GND', 'C')
two('C12', '10nF 50V X7R', (490, 243), 'LP2', 'GND', 'C')
two('R8', '100k 1%', (350, 267), 'LP2', 'AUDIO_QUIET')
two('R9', '1k 1%', (435, 267), 'AUDIO_QUIET', 'GND')
two('C13', '220nF 25V X7R', (530, 267), 'AUDIO_QUIET', 'AMP_INP', 'C')
note(320, 289, 'R8/R9 set a conservative starting volume. Tune by listening; do not assume a 3 W output.')

part('J2', 'AVR ISP 2x3', (65, 346), [
    (1,'MISO','passive','ISP_MISO'), (3,'SCK','passive','ISP_SCK'),
    (5,'RESET','passive','RESET_N'), (2,'VTREF','passive','+5V'),
    (4,'MOSI','passive','ISP_MOSI'), (6,'GND','passive','GND')],
    footprint='Connector_PinHeader_2.54mm:PinHeader_2x03_P2.54mm_Vertical',
    note='Programming/recovery access. USB powers target; programmer must not supply power.')
part('SW1', 'SKRPACE010', (160, 340), [
    (1,'Contact 1','passive','RESET_N'), (2,'Contact 2','passive','RESET_N'),
    (3,'Contact 3','passive','GND'), (4,'Contact 4','passive','GND')],
    footprint='AraBoard:SW_SKRPACE010', mpn='SKRPACE010', lcsc='C139797',
    note='Alps diagram: 1/2 are common; 3/4 are common. Press joins both groups.')
for ref, net, x, y in [('TP1','+5V',245,332), ('TP2','GND',245,346),
                       ('TP3','AUDIO_PWM',245,360), ('TP4','ADC0_SEED',245,374)]:
    part(ref, net, (x,y), [(1,net,'passive',net)],
         footprint='TestPoint:TestPoint_Pad_D1.5mm')
note(25, 380, 'Program bootloader and fuses through J2 before USB serial uploads can work.')
note(25, 386, 'Use Arduino Uno / ATmega328P / 16 MHz / Optiboot, 115200 baud.')
note(25, 392, 'Do not connect a powered ISP source and USB power together.')

part('U3', 'PAM8302AADCR', (360, 344), [
    (3,'IN+','input','AMP_INP'), (4,'IN-','input','AMP_INM'),
    (1,'SD / enable','input','AMP_ENABLE'), (2,'NC','no_connect',None),
    (5,'VO+','output','AMP_OUTP'), (8,'VO-','output','AMP_OUTM'),
    (6,'VDD','power_in','+5V'), (7,'GND','power_in','GND')],
    footprint='Package_SO:SOIC-8_3.9x4.9mm_P1.27mm', mpn='PAM8302AADCR')
two('FB2', 'BLM21PG221SZ1D', (445, 330), 'AMP_OUTP', 'SPK_P', 'FB', mpn='BLM21PG221SZ1D')
two('FB3', 'BLM21PG221SZ1D', (445, 350), 'AMP_OUTM', 'SPK_M', 'FB', mpn='BLM21PG221SZ1D')
part('J3', 'JST PH 2-pin', (540, 337), [
    (1,'Speaker +','passive','SPK_P'), (2,'Speaker -','passive','SPK_M')],
    footprint='Connector_JST:JST_PH_S2B-PH-K_1x02_P2.00mm_Horizontal',
    mpn='S2B-PH-K-S(LF)(SN)', note='Verify supplied cable polarity. Neither speaker lead is ground.')
two('C14', '220nF 25V X7R', (350, 371), 'GND', 'AMP_INM', 'C')
two('C15', '1uF 50V X5R', (440, 371), '+5V', 'GND', 'C')
two('C16', '22uF 25V X5R', (535, 371), '+5V', 'GND', 'C',
    footprint='Capacitor_SMD:C_1206_3216Metric', note='Verify effective capacitance at 5 V.')
two('R10', '100k 1%', (350, 390), '+5V', 'AMP_ENABLE')
two('C17', '100nF 50V X7R', (440, 390), 'AMP_ENABLE', 'GND', 'C')
two('R11', '1k 1%', (535, 390), 'AMP_CONTROL', 'AMP_ENABLE')
two('C18', '1nF 50V X7R', (540, 350), 'SPK_P', 'GND', 'C')
# The second EMI capacitor is grouped in the filter panel to preserve drawing clearance.
two('C19', '1nF 50V X7R', (540, 285), 'SPK_M', 'GND', 'C')
note(320, 407, 'Bridge output: neither speaker pin connects to GND. Keep speaker pair short / twisted.')
for ref,net,x,y in [('#FLG01','VBUS',150,151),('#FLG02','GND',235,151),
                    ('#FLG03','USB_5V',150,166),('#FLG04','AVCC',510,175)]:
    part(ref,'PWR_FLAG',(x,y),[(1,'pwr','power_out',net)],kind='flag')

# Exact procurement parts selected from the dated JLCPCB snapshot.
selection = json.loads((HERE.parent/'pcb'/'parts.json').read_text())
by_code = json.loads((HERE.parent/'pcb'/'sourcing.json').read_text())['parts']
for group in selection['groups']:
    source = by_code[group['lcsc']]
    for ref in group['references']:
        p = next(p for p in PARTS if p['ref'] == ref)
        p['mpn'] = source['componentModelEn']
        p['lcsc'] = group['lcsc']

def effects(size=1.27, justify='', hide=False):
    return f'(effects (font (size {size} {size})){" (justify " + justify + ")" if justify else ""}{" hide" if hide else ""})'

def libsymbol(p):
    name = 'Ara:' + p['ref']
    w,h = p['width']/2,p['height']/2
    body = f'(rectangle (start {-w} {h}) (end {w} {-h}) (stroke (width 0.254) (type default)) (fill (type background)))'
    virtual=p['kind']=='flag'
    if virtual:
        body='(polyline (pts (xy 0 0) (xy -1.27 1.27) (xy 0 2.54) (xy 1.27 1.27) (xy 0 0)) (stroke (width 0.254) (type default)) (fill (type none)))'
    elif p['kind'] == 'C':
        body = ''.join(f'(polyline (pts (xy {a} {b}) (xy {c} {d})) (stroke (width 0.254) (type default)) (fill (type none)))'
                       for a,b,c,d in [(-5.08,0,-1.27,0),(-1.27,-2.54,-1.27,2.54),(1.27,-2.54,1.27,2.54),(1.27,0,5.08,0)])
    elif p['kind'] == 'D':
        body = ''.join(f'(polyline (pts {points}) (stroke (width 0.254) (type default)) (fill (type none)))'
                       for points in ['(xy -3.175 0) (xy -1.27 0)',
                                      '(xy -1.27 -2.54) (xy -1.27 2.54)',
                                      '(xy -1.27 0) (xy 1.27 2.54) (xy 1.27 -2.54) (xy -1.27 0)',
                                      '(xy 1.27 0) (xy 3.175 0)'])
    s = f'(symbol {q(name)} {"(power)" if virtual else ""} (pin_names (offset 0.762)) (in_bom {"no" if virtual or p["ref"].startswith("TP") else "yes"}) (on_board {"no" if virtual else "yes"})\n'
    s += f'(property "Reference" {q(p["ref"])} (id 0) (at 0 {h+3.81} 0) {effects()})\n'
    s += f'(property "Value" {q(p["value"])} (id 1) (at 0 {h+1.27} 0) {effects()})\n'
    s += f'(symbol {q(p["ref"]+"_0_1")} {body})\n'
    s += f'(symbol {q(p["ref"]+"_1_1")}\n'
    for pin in p['pins']:
        length = 2.54 if p['kind']=='box' else 1.905
        if p['kind'] in ('C','flag'): length=0
        s += f'(pin {pin["type"]} line (at {pin["dx"]} {-pin["dy"]} {0 if pin["side"]=="L" else 180}) (length {length}) '
        s += f'(name {q(pin["name"])} {effects(1.016,hide=p["kind"]!="box")}) (number {q(pin["number"])} {effects(1.016,hide=virtual)}))\n'
    return s + '))\n'

def kicad():
    s = '(kicad_sch (version 20211123) (generator ara_schematic_generator)\n'
    s += f'(uuid {ROOT}) (paper "User" 620 460)\n'
    s += '(title_block (title "Ara - Arduino audio board") (date "2026-11-25") (rev "A-prototype") (comment 1 "PROTOTYPE - PHYSICAL TESTING PENDING"))\n'
    s += '(lib_symbols\n' + ''.join(libsymbol(p) for p in PARTS) + ')\n'
    for p in PARTS:
        ref,x,y=p['ref'],p['x'],p['y']
        virtual=p['kind']=='flag'
        top=y-p['height']/2
        s += f'(symbol (lib_id {q("Ara:"+ref)}) (at {x} {y} 0) (unit 1) (in_bom {"no" if virtual or p["ref"].startswith("TP") else "yes"}) (on_board {"no" if virtual else "yes"}) (uuid {uid(ref)})\n'
        for i,(key,val,py,hide) in enumerate([
            ('Reference',ref,top-4.0,virtual),('Value',p['value'],top-1.4,False),
            ('Footprint',p['footprint'],y,True),('Datasheet','',y,True),
            ('MPN',p['mpn'],y,True),('LCSC',p['lcsc'],y,True),('DesignNote',p['note'],y,True)]):
            s += f'(property {q(key)} {q(val)} (id {i}) (at {x} {py} 0) {effects(1.27,hide=hide)})\n'
        for pin in p['pins']:
            s += f'(pin {q(pin["number"])} (uuid {uid(ref+"/"+pin["number"])}))\n'
        s += ')\n'
        for pin in p['pins']:
            px,py=round(x+pin['dx'],4),round(y+pin['dy'],4)
            if pin['net'] is None:
                s += f'(no_connect (at {px} {py}) (uuid {uid(ref+pin["number"]+"nc")}))\n'
            else:
                end=round(px+(-3.81 if pin['side']=='L' else 3.81),4)
                s += f'(wire (pts (xy {px} {py}) (xy {end} {py})) (stroke (width 0) (type default)) (uuid {uid(ref+pin["number"]+"wire")}))\n'
                s += f'(label {q(pin["net"])} (at {end} {py} {180 if pin["side"]=="L" else 0}) {effects(1.016,"left bottom")} (uuid {uid(ref+pin["number"]+"label")}))\n'
    for x,y,w,h,title in PANELS:
        s += f'(text {q(title)} (at {x+3} {y+5} 0) {effects(2,"left")} (uuid {uid(title)}))\n'
    for x,y,text in NOTES:
        s += f'(text {q(text)} (at {x} {y} 0) {effects(1.27,"left")} (uuid {uid(text)}))\n'
    s += '(sheet_instances (path "/" (page "1")))\n(symbol_instances\n'
    for p in PARTS:
        s += f'(path "/{uid(p["ref"])}" (reference {q(p["ref"])}) (unit 1) (value {q(p["value"])}) (footprint {q(p["footprint"])}))\n'
    return s+'))\n'

def svg():
    el=[]
    def line(x,y,a,b,color='#277858',width=.25):
        el.append(f'<path d="M{x},{y} L{a},{b}" stroke="{color}" stroke-width="{width}" fill="none"/>')
    def text(x,y,t,size=1.3,anchor='middle',color='#1f2937'):
        el.append(f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" fill="{color}">{html.escape(t)}</text>')
    def rect(x,y,w,h,stroke='#26394b',fill='white',width=.25):
        el.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" stroke="{stroke}" stroke-width="{width}" fill="{fill}"/>')
    rect(0,0,594,420,fill='#ffffff')
    text(12,17,'ARA / Arduino audio board',5,'start')
    text(12,25,'Revision A prototype | 25 November 2026 | Same-name labels are electrically connected.',2,'start')
    text(580,17,'PROTOTYPE DESIGN',3,'end',color='#a53623')
    text(580,24,'Physical testing pending',2,'end',color='#a53623')
    for x,y,w,h,title in PANELS:
        rect(x,y,w,h,stroke='#b4c2cc',fill='#fbfcfd')
        text(x+3,y+6,title,2.6,'start')
    for p in PARTS:
        x,y,w,h=p['x'],p['y'],p['width'],p['height']
        if p['kind']!='flag':text(x,y-h/2-3.8,p['ref'],1.7)
        text(x,y-h/2-1.1,p['value'],1.35)
        if p['kind']=='flag':
            for a,b,c,d in [(0,0,-1.27,-1.27),(-1.27,-1.27,0,-2.54),(0,-2.54,1.27,-1.27),(1.27,-1.27,0,0)]:
                line(x+a,y+b,x+c,y+d,'#26394b')
        elif p['kind']=='C':
            for a in [-1.27,1.27]:line(x+a,y-2.54,x+a,y+2.54,'#26394b',.35)
            line(x-5.08,y,x-1.27,y);line(x+1.27,y,x+5.08,y)
        elif p['kind']=='D':
            line(x-5.08,y,x-1.27,y);line(x+1.27,y,x+5.08,y)
            line(x-1.27,y-2.54,x-1.27,y+2.54,'#26394b',.35)
            for a,b,c,d in [(-1.27,0,1.27,-2.54),(1.27,-2.54,1.27,2.54),(1.27,2.54,-1.27,0)]:
                line(x+a,y+b,x+c,y+d,'#26394b',.35)
        else:
            rect(x-w/2,y-h/2,w,h,fill='#fffef7' if p['kind']=='box' else 'white')
        for pin in p['pins']:
            px,py=x+pin['dx'],y+pin['dy'];left=pin['side']=='L'
            if p['kind']=='box':
                line(px,py,x+(-w/2 if left else w/2),py,'#26394b')
                text(x+(-w/2+1 if left else w/2-1),py+.4,pin['name'],1.05,'start' if left else 'end')
            elif p['kind'] not in ('C','D','flag'):
                line(px,py,x+(-w/2 if left else w/2),py,'#26394b')
            if p['kind']!='flag':text(px+(1 if left else -1),py-.55,pin['number'],.85,'start' if left else 'end',color='#7b4454')
            if pin['net'] is None:
                line(px-.7,py-.7,px+.7,py+.7,'#617d9f');line(px-.7,py+.7,px+.7,py-.7,'#617d9f')
            else:
                end=px+(-3.81 if left else 3.81)
                line(px,py,end,py)
                text(end,py-.7,pin['net'],1.08,'end' if left else 'start',color='#166145')
    for x,y,t in NOTES:text(x,y,t,1.45,'start',color='#425568')
    start='<svg xmlns="http://www.w3.org/2000/svg" width="2970" height="2100" viewBox="0 0 594 420"><g font-family="Arial, Helvetica, sans-serif">'
    return start+''.join(el)+'</g></svg>'

def netlist():
    root=ET.Element('export',version='E')
    design=ET.SubElement(root,'design');ET.SubElement(design,'source').text='ara.kicad_sch'
    components=ET.SubElement(root,'components')
    nets={}
    for p in PARTS:
        if p['kind']!='flag':
            c=ET.SubElement(components,'comp',ref=p['ref'])
            ET.SubElement(c,'value').text=p['value'];ET.SubElement(c,'footprint').text=p['footprint']
        for pin in p['pins']:
            if pin['net'] is not None and p['kind']!='flag':nets.setdefault(pin['net'],[]).append((p['ref'],pin))
    nn=ET.SubElement(root,'nets')
    for code,(name,nodes) in enumerate(sorted(nets.items()),1):
        n=ET.SubElement(nn,'net',code=str(code),name=name)
        for ref,pin in nodes:ET.SubElement(n,'node',ref=ref,pin=pin['number'],pinfunction=pin['name'],pintype=pin['type'])
    ET.indent(root)
    return ET.tostring(root,encoding='unicode')+'\n'

if __name__=='__main__':
    HERE.mkdir(parents=True,exist_ok=True)
    (HERE/'ara.kicad_sch').write_text(kicad())
    (HERE/'ara.svg').write_text(svg())
    (HERE/'ara.net').write_text(netlist())
    (HERE/'circuit.json').write_text(json.dumps(PARTS,indent=2)+'\n')
    # Keep the custom symbols available to native ERC and future editing.
    library = ''.join(libsymbol(p).replace(q('Ara:'+p['ref']), q(p['ref']), 1) for p in PARTS)
    (HERE/'Ara.kicad_sym').write_text('(kicad_symbol_lib (version 20211014) (generator ara_schematic_generator)\n'+library+')\n')
    (HERE/'sym-lib-table').write_text('(sym_lib_table (lib (name "Ara") (type "KiCad") (uri "${KIPRJMOD}/Ara.kicad_sym") (options "") (descr "Ara circuit symbols")))\n')
    print(f'Generated {len(PARTS)} components in {HERE}')
