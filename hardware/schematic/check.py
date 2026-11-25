#!/usr/bin/env python3
"""Check actual schematic connectivity and electrical design invariants.

Uses only Python's standard library. This is not a replacement for KiCad ERC,
layout review, simulation or measurements on an assembled board.
"""
import json
import math
from pathlib import Path
import re
import xml.etree.ElementTree as ET

HERE = Path(__file__).resolve().parent

def parse(text):
    tokens = re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^\s()]+', text)
    stack, root = [], []
    current = root
    for token in tokens:
        if token == '(':
            new=[];current.append(new);stack.append(current);current=new
        elif token == ')':
            assert stack, 'Unbalanced closing parenthesis'
            current=stack.pop()
        else:
            current.append(json.loads(token) if token.startswith('"') else token)
    assert not stack and len(root)==1, 'Unbalanced schematic expression'
    return root[0]

def children(node, key):
    return [n for n in node if isinstance(n,list) and n and n[0]==key]

def child(node, key):
    result=children(node,key)
    assert result, f'Missing {key}'
    return result[0]

def point(node):
    return tuple(round(float(v),4) for v in node[1:3])

tree=parse((HERE/'ara.kicad_sch').read_text())
assert tree[0]=='kicad_sch'
libraries={n[1]:n for n in children(child(tree,'lib_symbols'),'symbol')}
parents={}
def find(p):
    parents.setdefault(p,p)
    if parents[p]!=p:parents[p]=find(parents[p])
    return parents[p]
def union(a,b):parents[find(a)]=find(b)

wires=[]
for w in children(tree,'wire'):
    a,b=[point(p) for p in child(w,'pts')[1:]]
    union(a,b);wires.append((a,b))
labels={}
for label in children(tree,'label'):
    pt=point(child(label,'at'));name=label[1]
    assert pt not in labels or labels[pt]==name, 'Different labels at one point'
    labels[pt]=name
    # A pin or label in the interior of a segment is electrically connected too.
    for a,b in wires:
        if (a[1]==b[1]==pt[1] and min(a[0],b[0])<=pt[0]<=max(a[0],b[0])) or \
           (a[0]==b[0]==pt[0] and min(a[1],b[1])<=pt[1]<=max(a[1],b[1])):
            union(pt,a)
net_by_node={}
for pt,name in labels.items():
    group=find(pt)
    assert group not in net_by_node or net_by_node[group]==name, f'Shorted named nets at {pt}'
    net_by_node[group]=name
nc={point(child(n,'at')) for n in children(tree,'no_connect')}
components={};pins={};pin_names={};positions=set()
for s in children(tree,'symbol'):
    props={p[1]:p[2] for p in children(s,'property')}
    ref=props['Reference'];assert ref not in components, f'Duplicate {ref}'
    components[ref]=props
    origin=point(child(s,'at'));lib=libraries[child(s,'lib_id')[1]]
    for unit in children(lib,'symbol'):
        for pin in children(unit,'pin'):
            number=child(pin,'number')[1];name=child(pin,'name')[1]
            px,py=point(child(pin,'at'))
            pos=round(origin[0]+px,4),round(origin[1]-py,4)
            assert pos not in positions, f'Overlapping pins at {pos}'
            positions.add(pos)
            net=net_by_node.get(find(pos))
            assert (net is not None) != (pos in nc), f'{ref}.{number} needs a net or an NC marker'
            pins[ref,number]=net;pin_names[ref,number]=name

def net(ref,pin):return pins[ref,str(pin)]
def same(expected,*ends):
    assert all(net(r,p)==expected for r,p in ends), f'{expected}: {ends}'
def pair(ref,a,b):
    assert {net(ref,1),net(ref,2)}=={a,b}, f'{ref}: {a}, {b}'
def value(ref,v):assert components[ref]['Value']==v, f'{ref} value'

# Pin-level checks independent of the generator's part declarations.
assert len([p for p in pins if p[0]=='U1'])==32
for number,name in {2:'PD4 / D4',4:'VCC',6:'VCC',7:'PB6 / XTAL1',8:'PB7 / XTAL2',
                    13:'PB1 / D9 PWM',15:'PB3 / MOSI',16:'PB4 / MISO',17:'PB5 / SCK',
                    18:'AVCC',20:'AREF',23:'PC0 / A0',29:'PC6 / RESET',
                    30:'PD0 / RXD',31:'PD1 / TXD'}.items():
    assert pin_names['U1',str(number)]==name, f'AVR package mismatch on pin {number}'
same('+5V',('U1',4),('U1',6),('U2',16),('U3',6),('U4',6),('U4',5))
same('GND',('U1',3),('U1',5),('U1',21),('U2',1),('U2',9),('U2',15),('U3',7),('U4',2))
same('VBUS',*[('J1',p) for p in ['A4','A9','B4','B9']],('F1',1),('U5',5))
same('GND',*[('J1',p) for p in ['A1','A12','B1','B12','SH']])
same('USB_DP',('J1','A6'),('J1','B6'),('U2',5),('U5',1),('U5',6))
same('USB_DM',('J1','A7'),('J1','B7'),('U2',6),('U5',3),('U5',4))
pair('R1','CC1','GND');pair('R2','CC2','GND')
value('R1','5.1k 1%');value('R2','5.1k 1%')
assert net('J1','A5')!=net('J1','B5')
pair('F1','VBUS','USB_5V');same('USB_5V',('U4',1),('U4',3))
pair('C2','SOFTSTART','GND');same('SOFTSTART',('U4',4))
same('AVCC',('U1',18),('FB1',2),('C6',1))
pair('C7','AREF','GND');assert net('U1',20)=='AREF'
same('XTAL1',('U1',7),('Y1',1));same('XTAL2',('U1',8),('Y1',3));assert net('Y1',2)=='GND'
pair('R4','USB_TX','MCU_RX');pair('R5','MCU_TX','USB_RX')
same('USB_TX',('U2',2));same('USB_RX',('U2',3))
same('MCU_RX',('U1',30));same('MCU_TX',('U1',31))
pair('C9','CH340_V3','GND');assert net('U2',4)=='CH340_V3'
pair('C10','DTR_N','RESET_N');assert net('U2',13)=='DTR_N'
same('RESET_N',('U1',29),('D1',2),('SW1',1),('J2',5))
assert net('D1',1)=='+5V' and pin_names['D1','1']=='K', 'Reset diode reversed'
for pin,expected in {1:'ISP_MISO',2:'+5V',3:'ISP_SCK',4:'ISP_MOSI',5:'RESET_N',6:'GND'}.items():
    assert net('J2',pin)==expected
same('AUDIO_PWM',('U1',13),('R6',1),('TP3',1))
pair('R6','AUDIO_PWM','LP1');pair('C11','LP1','GND')
pair('R7','LP1','LP2');pair('C12','LP2','GND')
pair('R8','LP2','AUDIO_QUIET');pair('R9','AUDIO_QUIET','GND')
pair('C13','AUDIO_QUIET','AMP_INP');assert net('U3',3)=='AMP_INP'
pair('C14','GND','AMP_INM');assert net('U3',4)=='AMP_INM'
pair('R11','AMP_CONTROL','AMP_ENABLE');same('AMP_CONTROL',('U1',2))
same('AMP_ENABLE',('U3',1),('R10',2),('C17',1))
pair('FB2','AMP_OUTP','SPK_P');pair('FB3','AMP_OUTM','SPK_M')
assert net('U3',5)=='AMP_OUTP' and net('U3',8)=='AMP_OUTM'
assert net('J3',1)=='SPK_P' and net('J3',2)=='SPK_M'
assert net('J3',1)!=net('J3',2) and 'GND' not in [net('J3',1),net('J3',2)]
assert {p for p,n in pins.items() if n=='ADC0_SEED'}=={('U1','23'),('TP4','1')}
for name in set(pins.values())-{None}:
    assert sum(n==name for n in pins.values())>=2, f'Dangling net {name}'

# The supplied .net must agree with connections reconstructed from native KiCad geometry.
xml=ET.parse(HERE/'ara.net').getroot()
exported={}
for n in xml.find('nets'):
    for p in n:exported[p.attrib['ref'],p.attrib['pin']]=n.attrib['name']
assert exported=={p:n for p,n in pins.items() if n is not None and not p[0].startswith('#')}, 'Exported netlist diverged'
physical_components={r:p for r,p in components.items() if not r.startswith('#')}
assert len(xml.find('components'))==len(physical_components)
uuids=re.findall(r'\(uuid ([^\s)]+)\)',(HERE/'ara.kicad_sch').read_text())
assert len(uuids)==len(set(uuids)), 'Duplicate UUIDs'

# Linear small-signal calculation: two loaded RC stages, divider and 10k amp input.
def response(f):
    jw=2j*math.pi*f
    zinput=10000+1/(jw*220e-9)
    zq=1/(1/1000+1/zinput)
    z2=1/(jw*10e-9+1/(100000+zq))
    z1=1/(jw*22e-9+1/(2200+z2))
    return z1/(1000+z1)*z2/(2200+z2)*zq/(100000+zq)*10000/zinput*15
samples={str(f):round(20*math.log10(abs(response(f))),2) for f in [500,1000,3000,6000,10000,32768]}
results={
    'status':'PASS: connectivity and design-invariant checks; not KiCad ERC',
    'components':len(physical_components),'power_flags':len(components)-len(physical_components),
    'connected_pins':len(exported),
    'nets':len(set(exported.values())), 'unconnected_pins_with_markers':len(nc),
    'small_signal_gain_db_pwm_to_speaker':samples,
    'outside_this_check':['KiCad native ERC','PCB layout DRC','Circuit simulation','Physical measurements'],
}
(HERE/'checks.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))
