# Amplified Futures — Feature Backlog

## Pending

### Panel / visual
- [ ] **SVG text-to-paths** — Open each panel SVG in Inkscape → select all text → `Path > Object to Path` → save as Plain SVG. Converts `<text>` elements to bezier outlines for font-independent rendering. Affects all 11 module SVGs.
- [ ] **Batch Inkscape export** — run `amplified-futures/vcv-modules/scripts/graphics/export-panels.ps1` against all 11 existing module SVGs once text-to-paths pass is done.

### Shipping / library prep
- [ ] **Per-module manuals** — `docs/modules/<slug>.md` for each of the 11 modules: controls reference, signal flow, patch tips.
- [ ] **Factory presets** — `.vcvs` JSON preset files in `presets/amplified-futures/<slug>/`. 2–3 per module minimum.
- [ ] **How-to guides** — `docs/guides/` short task-focused docs.
- [ ] **Playbooks** — `docs/playbooks/` named patch configurations: The Wall, Drone Bed, Feedback Republic, Harmonic Pressure Session, Percussion Slab.

### Modules
- [ ] **Mass Driver C++** (AF-01, 32HP) — implement `src/MassDriver.cpp` from SVG panel + coordinate JSON already designed in `amplified-futures/vcv-modules/design/`. 16-channel mixer, per-channel GAIN + MUTE, master DENSITY/PRESSURE/WIDTH/FEEDBACK/COLLAPSE.
- [ ] **State serialisation** — `dataToJson` / `dataFromJson` for: Pulse step grid, StringMassCore voice phases, WallConductor collapse state.
- [ ] **Dark panel variants** — all 11 modules.

### DSP polish
- [ ] **DroneClone voice LEDs** — confirm they respond correctly to amplitude envelope, not just gate state.
- [ ] **DroneClone RTN DC check** — verify feedback return path has DC blocker (same pattern as FeedbackGovernor).

---

## Completed

- [x] 11 modules built and installed: DroneCore, DroneClone, Send, Choke, Pulse, Drift, WallConductor, StringMassCore, HarmonicPressure, CollapseSat, FeedbackGovernor
- [x] All modules: every knob has CV input + attenuverter (satellite layout)
- [x] All modules: V/OCT IN → V/OCT OUT pass-through
- [x] All modules: module title rendered by C++ NanoVG in safety orange header
- [x] Amplified Futures design system applied across all modules
- [x] Mass Driver SVG panel designed (32HP) + coordinate JSON + PNG/PDF exported via Inkscape
- [x] `amplified-futures/vcv-modules/design/panels/style/amplified-futures-panel-style.md` — full design system doc
- [x] `amplified-futures/vcv-modules/scripts/graphics/export-panels.ps1` — batch Inkscape export script
- [x] CLAUDE.md, AGENTS.md, `.github/copilot-instructions.md` created (2026-05-19)
