# Branca Rack Modules

**Amplified Futures** — eleven VCV Rack 2 modules for dense experimental sound. Massed oscillators, controlled feedback, no-wave rhythmics, microtonal pressure. Branca/noise-rock genealogy; built for live performance.

---

## Modules

### DRONECORE — 8HP
Two-voice detuned oscillator core. PITCH, DETUNE (0–100¢ spread), TIMBRE (sine to third-bridge harmonic stack). Fully polyphonic. Every knob has CV + attenuverter. V/OCT pass-through.

### DRONECLONE — 22HP
8-voice amplified string wall. MASS (active voice count), TENSION (harmonic edge), SHIMMER (air), JAWARI (rattle/buzz), WEIGHT (sub body), DRIFT (per-voice wander). CHOKE button/gate collapses the wall. RTN feedback input for self-patching loops. Polyphonic (up to 16 poly channels × 8 voices = 128 simultaneous oscillators).

### SEND — 12HP
2×2 cross-send feedback routing matrix. A→B send, B→A return, A→C/C→A internal feedback bus. One-sample delayed C-bus for safe self-oscillation. Polyphonic.

### CHOKE — 14HP
4-channel mixer built as an instrument. GAIN + TONE per channel. Fixed auto-pan spread (L / L–C / R–C / R). MUTE buttons. MAIN master with soft saturation. Stereo L/R out.

### PULSE — 12HP
16-step no-wave step percussion. 4×4 toggle grid. White noise synthesis: HIT level, DECAY time (8–500 ms), METAL (LP filter 360→80 Hz), CRACK (4 ms transient burst). TRG clock in, audio out.

### DRIFT — 12HP
Slow random modulation source. RATE (0.01–10 Hz exponential), WANDER (random walk step size), SLEW (1000→0.1 Hz LP). SYNC input forces immediate step. SMOOTH output (slewed ±5V), STEP output (raw ±5V), GATE output (10V 5ms pulse on each step). V/OCT pass-through.

### WALL CONDUCTOR — 22HP
Section-based performance mixer/conductor. DENSITY sweeps 4 channels in. PRESSURE saturates. WIDTH spreads stereo field. FEEDBACK loop (1-sample safe). COLLAPSE gate with shaped RECOVERY. Master stereo out.

### STRING MASS CORE — 16HP
16-voice harmonic mass oscillator. MASS (voice count), SPREAD (detune), TIMBRE (harmonic content). Four modes: UNIS (unison), HARM (Branca odd-harmonic sections), JUST (Ptolemaic JI ratios), MICRO (spectral microtonality). 1/√N amplitude normalised. Polyphonic V/OCT in → audio out.

### HARMONIC PRESSURE — 14HP
Harmonic series pitch CV generator. PITCH root + SPREAD ensemble detuning. PARTIAL selects first partial, COUNT sets how many (polyphonic channels). JUST / EQUAL / MICRO tuning modes. Outputs polyphonic V/oct for String Mass Core.

### COLLAPSE SATURATOR — 12HP
Stereo drive/saturation with collapse. DRIVE pre-gain. BUZZ character switch: ODD (symmetric tanh), EVEN (asymmetric tape), FULL (hard clip). COLLAPSE gate instantly maxes drive; RECOVERY sets return time. Sidechain input boosts drive. V/OCT pass-through.

### FEEDBACK GOVERNOR — 12HP
Controlled feedback send/return. AMOUNT level. TONE LP filter (100Hz → 20kHz). DECAY attenuates per pass (`amount × 0.5^(4×decay)`). KILL button/gate zeros the feedback path immediately. DC blocker + ±10V safety limiter. V/OCT pass-through.

---

## Design

All modules share:
- **CV + attenuverter** on every knob (satellite layout — trimpot + jack adjacent to knob)
- **V/OCT pass-through** — mono or poly on every module
- **Amplified Futures design system** — safety orange constructivist header (`#FF4A0E`), dark steel (`#1F2A1F`), colour-coded jacks (yellow CV, green V/OCT, orange audio)
- **Module title** drawn via C++ NanoVG (not SVG text)

---

## Build

Requires VCV Rack 2 SDK and MSYS2 MinGW64 (Windows).

```powershell
& "D:\dev-vcv\msys64\msys2_shell.cmd" -mingw64 -defterm -no-start -c "cd /d/dev-vcv/plugins/branca-rack-modules && RACK_DIR=/d/dev-vcv/Rack-SDK make -j4"
```

## Install

```powershell
Remove-Item "C:\Users\danie\AppData\Local\Rack2\plugins-win-x64\branca-rack-modules" -Recurse -Force -ErrorAction SilentlyContinue
Copy-Item "D:\dev-vcv\plugins\branca-rack-modules" -Destination "C:\Users\danie\AppData\Local\Rack2\plugins-win-x64\" -Recurse
```

---

*Daniel Boles / Amplified Futures — 2026*
