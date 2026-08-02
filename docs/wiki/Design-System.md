# Design System

The Amplified Futures panel language — colours, grid, typography, and widget layout conventions shared across all 19 modules.

---

## Colour palette

| Role | Name | Hex | Used for |
|---|---|---|---|
| Header bar | Safety Orange | `#FF4A0E` | Module header background |
| Panel body | Dark Steel | `#1F2A1F` | Panel background (most modules) |
| CV jack stroke | Signal Yellow | `#E8C547` | Attenuverter + CV input jacks |
| V/OCT jack stroke | Thru Green | `#5A7350` | V/OCT input and thru-output jacks |
| Audio jack stroke | Ember | `#FF4A0E` | Audio input/output jacks |
| Audio jack fill | Ember fill | `#201808` | Audio jack fill |
| Primary label | Paper | `#C8C0B0` | Control labels, parameter names |
| Secondary label | Concrete | `#887860` | Range labels, units, dim text |

Colour-coded jacks let you identify signal types at a glance across a busy rack:
- **Yellow** = CV/modulation signal
- **Green** = V/OCT pitch signal  
- **Orange/dark** = audio signal

---

## HP grid

All module widths are multiples of 1HP (horizontal pitch). One panel unit:

| Unit | Pixels | Millimetres |
|---|---|---|
| 1 HP | 15.24 px | 5.08 mm |
| 1 mm | 3 px | — |
| Panel height | 380 px | 128.5 mm |

Module widths: 8HP, 12HP, 14HP, 16HP, 18HP, 22HP, 32HP.

---

## Widget layout conventions

### Satellite layout — attenuverter + CV adjacent to each knob

Every main parameter knob that accepts CV has a **Trimpot** (attenuverter, radius 4) and a **PJ301M** (CV jack, radius 5) positioned adjacent to it. Swarm Core DECAY intentionally omits the attenuverter.

**Vertical layout** (stacked below knob):
- Attenuverter at `(kx, ky + 18mm)`
- CV jack at `(kx, ky + 35.4mm)`

**Horizontal layout** (Drift-style, either side of knob):
- Attenuverter at `(kx − 5.9mm, ky + 11.8mm)`
- CV jack at `(kx + 5.9mm, ky + 11.8mm)`

### Jack colour convention by SVG stroke

| Signal type | Stroke colour | Jack type |
|---|---|---|
| Audio in/out | Ember `#FF4A0E` | PJ301M |
| V/OCT | Thru Green `#5A7350` | PJ301M |
| CV/modulation | Signal Yellow `#E8C547` | PJ301M |
| Gate/trigger | Signal Yellow `#E8C547` | PJ301M |

---

## Typography

**Module name**: drawn via C++ NanoVG in the `draw()` method — not SVG `<text>`. This is because nanosvg in VCV Rack does not render `<text>` elements with external fonts. The name appears correct in the rendered module.

**All other labels** (control labels, port labels, range values): written as SVG `<text>` elements — nanosvg renders these correctly as simple strings.

**Text-to-paths**: Before exporting panels for the VCV Library, run Inkscape's text-to-paths pass (Object → Objects to Paths) to convert all SVG text to bezier outlines. This ensures font-independent rendering on any system.

---

## SVG export

```powershell
& "C:\Program Files\Inkscape\bin\inkscape.exe" `
  --export-type=png `
  --export-dpi=96 `
  --export-filename=out.png `
  in.svg
```

For batch export of all panels, run `scripts/graphics/export-panels.ps1`.

---

## See also

[[Home]] · [[Building-from-Source]] · [[Module-Reference]]
