# Amplified Futures — Feature Backlog

## Pending

### Shipping / library prep
- [x] **Factory presets** — 57 `.vcvm` files in `presets/<slug>/`, at least two per module. Shipped in 2.3.0.
- [ ] **How-to guides** — `docs/guides/` short task-focused docs.

### Modules
- [ ] **State serialisation** — `dataToJson` / `dataFromJson` for: Pulse step grid, StringMassCore voice phases, WallConductor collapse state.
- [ ] **Dark panel variants** — all 19 modules.

### DSP polish
- [ ] **DroneClone voice LEDs** — confirm they respond correctly to amplitude envelope, not just gate state.
- [ ] **DroneClone RTN DC check** — verify feedback return path has DC blocker (same pattern as FeedbackGovernor).

### Distribution
- [ ] **macOS / Linux release binaries** — cross-building is not set up, so releases are Windows x64 only. Source builds work on all platforms.

### Panel imagery
- [ ] **Rack-rendered screenshots for the utility modules** — `docs/panels/rack/` still has no render for Collapse EG, Quad VCA, Ratchet or Signal Bloc. Their wiki pages carry text and a flat export only, which makes them the odd ones out; every other module page leads with a Rack render.

---

## Completed

- [x] **19 modules built and installed**: DroneCore, DroneClone, Send, Choke, Pulse, Drift, WallConductor, StringMassCore, HarmonicPressure, CollapseSat, FeedbackGovernor, MassDriver, SitarGrid, SwarmCore, StreetGridClock, CollapseEG, QuadVCA, Ratchet, SignalBloc
- [x] **Mass Driver C++** (AF-01, 32HP) — `src/MassDriver.cpp`. 16-channel mixer, per-channel GAIN + MUTE, master DENSITY/PRESSURE/WIDTH/MASS/FEEDBACK/COLLAPSE, OUT + AUX + SUM outputs
- [x] **SVG text-to-paths** — all 19 module SVGs converted to bezier outlines for font-independent nanosvg rendering
- [x] **Batch Inkscape export** — flat panel PNGs exported for all 19 modules
- [x] **Panel repair pass (v2.1.0)** — label collisions resolved across the range; Choke and SwarmCore widened to 18 HP
- [x] **Rack-rendered screenshots** — `docs/panels/rack/` for 15 modules, plus a contact sheet
- [x] **Per-module manuals** — `docs/modules/<slug>.md` for all 19: controls reference, signal flow, MIDI CC map, patch tips
- [x] **Playbooks** — `docs/playbooks/`: The Wall, Drone Bed, Feedback Republic, Harmonic Pressure Session, Percussion Slab
- [x] **Wiki as repo source** — `docs/wiki/` is authoritative, published one-way by `tools/sync_wiki.ps1`; `tools/check_wiki.py` verifies links, HP and module coverage
- [x] All modules: every CV-able knob uses the satellite layout; Swarm Core DECAY is the documented exception
- [x] All modules: V/OCT IN → V/OCT OUT pass-through
- [x] All modules: module title rendered by C++ NanoVG in safety orange header
- [x] Amplified Futures design system applied across all modules
- [x] Mass Driver SVG panel designed (32HP) + coordinate JSON + PNG/PDF exported via Inkscape
- [x] `amplified-futures/vcv-modules/design/panels/style/amplified-futures-panel-style.md` — full design system doc
- [x] `amplified-futures/vcv-modules/scripts/graphics/export-panels.ps1` — batch Inkscape export script
- [x] CLAUDE.md, AGENTS.md, `.github/copilot-instructions.md` created (2026-05-19)
