# Prototype cost worksheet

Checked on 25 November 2026. All figures below are US dollars. No order has been placed.

| Item | Five boards | Ten boards | Basis |
| --- | ---: | ---: | --- |
| Parts fitted to the boards | $24.94 | $49.88 | Selected JLCPCB parts at the dated snapshot's initial unit prices |
| Bare PCB fabrication | $5.20 | $6.20 | JLCPCB web form, 65 × 45 mm, two layers, 1.6 mm, green, lead-free HASL |
| Assembly, handling and part attrition | $72.72 | $95.61 | Economic PCBA, top side, all 27 selected types |
| Board and assembly subtotal | **$77.92** | **$101.81** | Excludes shipping, taxes and programming |
| Initial programming and startup test | Not yet quoted | Not yet quoted | Requires factory agreement on method and price |
| Shipping and taxes | Not yet confirmed | Not yet confirmed | Depends on the actual delivery destination and checkout options |

The selected parts cost $4.988 per fitted board before quantity discounts or factory purchasing requirements. This is not the component charge on a completed factory quote. Minimum purchase quantities and assembly losses can add parts. `sourcing.json` records those fields and the original price breaks.

The selection has 27 unique part numbers. JLCPCB's snapshot classifies 13 as extended. Its [published assembly FAQ](https://jlcpcb.com/help/article/pcb-assembly-faqs) lists an $8 setup fee, a $1.50 stencil, a $3 feeder fee per extended component type, $0.0017 per SMT joint, and separate hand-soldering charges. Preferred extended parts, service tiers and coupons affect the actual fees. Charging all 13 feeder types alone would add $39 per batch, rather than per board.

These setup charges mean a $5 bare-board quote does not imply a $5 assembled batch. The assembly subtotals above are actual web quotes, including components; do not add the separate parts estimate to them. The separate speaker, USB cable, power supply and enclosure are outside the PCB quote.

## Quotation progress

The owner authorized uploading the Gerber ZIP, BOM and CPL for five- and ten-board quotes. The Gerber upload completed and JLCPCB detected the correct 65 × 45 mm two-layer board. Assembly is selected for the top side.

The owner approved the assembly-service terms and signed in. The BOM and CPL were uploaded, and all 27 selected part types were matched. JLCPCB initially substituted another CH340C stock code; it was changed back to the specified C84681 before calculating the quote.

The five-board quote is $77.92, made up of $5.20 PCB fabrication and $72.72 assembly/components. Assembly includes $8.24 setup, $1.55 stencil, $26.25 components, $30.90 extended-component fees, $1.51 SMT assembly, $3.61 hand-soldering labour and $0.66 manual assembly. The factory preview exposed component orientation and connector-origin differences. The CPL was corrected using numbered supplier CAD pads, reuploaded, and the final preview inspected. Final factory DFM placement approval remains required. The ten-board subtotal is $101.81, comprising $6.20 fabrication and $95.61 assembly/components. The extra five boards add $23.89 to the quoted subtotal. No order or payment has been submitted.
