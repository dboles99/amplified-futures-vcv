# Swarm Core — 18 HP

Bio-acoustic insect sample engine. Loads WAV files from the InsectSet32 sample bank (cicadidae / orthoptera, CC-BY 4.0, Zenodo 7072196). SPECIMEN select, PITCH, DENSITY, SCATTER, DETUNE, DECAY. Two modes: Specimen (single-voice pitched playback) and Swarm (8-voice detuned, time-scattered stereo cloud).

---

## Signal flow

```text
[background thread loads WAVs from res/insects/insectset32/]
bankReady = false during load (2–5 s silence at first patch load)
bankReady = true → bank safe to read from audio thread

TRIG IN ──► [rising edge → fire voice(s)]
V/OCT IN ──► pitch offset (semitones, adds to PITCH knob)

SPECIMEN mode (MODE = 0):
    one voice, sample = bank[SPECIMEN × bankSize]
    speed = 2^((PITCH_semitones) / 12) × (bankSampleRate / hostSampleRate)
    env × DECAY coef per sample → silence

SWARM mode (MODE = 1):
    up to 8 voices, triggered in succession with SCATTER delay
    voice i: speed = base × 2^(DETUNE × detune_offset[i] / 12)
    pan spread: voicePan[8] = {−1, −0.71, −0.33, 0, 0, 0.33, 0.71, 1}
    active voices: ceil(DENSITY × 8)
    mix × INV_SQRT_N (= 1/√8 = 0.354) → OUT L / OUT R
```

If the sample folder is absent, falls back to a noise-burst so the module is always functional.

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| SPECIMEN | 0 | 0–1 | 0 | Sample select: maps linearly across loaded bank (up to 64 samples) |
| PITCH | 1 | −24 to +24 st | 0 | Pitch offset in semitones. 0 = playback at original rate |
| DENSITY | 2 | 0–1 | 0.5 | Swarm voice count: 0 = 1 voice, 1 = 8 voices. SPECIMEN mode: ignored |
| SCATTER | 3 | 0–1 | 0.1 | Timing scatter between swarm voices (0 = simultaneous, 1 = max delay) |
| DETUNE | 4 | 0–1 | 0.2 | Per-voice pitch spread in Swarm mode (0 = all same pitch) |
| DECAY | 5 | 0–1 | 0.5 | Playback envelope decay — 0 = fast fade, 1 = full sample length |
| PITCH ATT | 6 | −1 to +1 | 0 | Attenuverter for PITCH CV |
| DENSITY ATT | 7 | −1 to +1 | 0 | Attenuverter for DENSITY CV |
| SCATTER ATT | 8 | −1 to +1 | 0 | Attenuverter for SCATTER CV |
| DETUNE ATT | 9 | −1 to +1 | 0 | Attenuverter for DETUNE CV |
| MODE | 10 | Toggle | Specimen | Button toggles Specimen / Swarm. LED: off = Specimen, lit = Swarm |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| TRIG IN | Input | Gate | Rising edge triggers playback. In Swarm mode triggers all active voices with scatter delays |
| V/OCT IN | Input | CV | Pitch offset (V/OCT, adds to PITCH knob in semitone-equivalent) |
| DENSITY CV | Input | CV | CV for DENSITY (scaled by DENSITY ATT) |
| SCATTER CV | Input | CV | CV for SCATTER |
| DETUNE CV | Input | CV | CV for DETUNE |
| DECAY CV | Input | CV | Added directly to the DECAY knob (voltage / 10), no attenuverter |
| OUT L | Output | Audio | Left stereo output |
| OUT R | Output | Audio | Right stereo output |
| CV OUT | Output | CV | Summed voice envelope × 1/√8, scaled to 0–10 V — an envelope follower on the module's own output |

DECAY **does** have a CV jack, but no attenuverter — its voltage is divided by 10 and added straight to the knob value, then clamped. SPECIMEN and MODE have neither CV nor attenuverter.

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | SPECIMEN | CC 14 (sample select — sweep to browse insects) |
| 1 | PITCH | CC 15 |
| 2 | DENSITY | CC 16 |
| 3 | SCATTER | CC 17 |
| 4 | DETUNE | CC 18 |
| 5 | DECAY | CC 19 |
| 10 | MODE | CC 20 (value ≥ 64 = Swarm, < 64 = Specimen) |

For jog-wheel style SPECIMEN select: CC values 1–63 = CW (next sample), 65–127 = CCW (previous sample). Use this with a relative encoder for tactile browsing.

---

## Sample bank details

- Source: InsectSet32 (Zenodo 7072196), CC-BY 4.0
- Families: cicadidae (cicadas), orthoptera (crickets, grasshoppers, katydids)
- Bank loads up to 64 WAV files from `res/insects/insectset32/` (recursive, sorted)
- WAV format: 16-bit or float32 PCM, mono or stereo (stereo is read as mono from left channel)
- Maximum sample length loaded: 5 seconds (220,500 frames at 44.1 kHz)
- Load time: 2–5 seconds on first patch load — outputs silence during this window
- ACTIVE LED (orange): lit when bank is loaded and ready
- SWARM LED (orange): lit when MODE = Swarm

---

## Recommended configurations

**Single Cicada** — Specimen mode. SPECIMEN select to a cicada sample. PITCH 0. DECAY 0.7. Trigger from a slow Drift GATE at RATE 0.3 Hz. Single insect call at irregular intervals.

**Cricket Chorus** — Swarm mode. DENSITY 0.8 (6–7 voices). SCATTER 0.15 (slight time spread). DETUNE 0.3 (pitch variation between voices). DECAY 0.6. Trigger from a regular clock at 120 BPM. Dense cricket chorus field.

**Pitched Texture** — Specimen mode. V/OCT IN from HarmonicPressure (4 partials). PITCH 0. DECAY 0.8. Each polyphonic trigger plays the same specimen at 4 different pitches simultaneously (use a poly trigger source). Insect sample as pitched harmonic texture.

**Storm Swarm** — Swarm mode. DENSITY 1.0 (all 8 voices). SCATTER 0.4 (large time scatter). DETUNE 0.6 (wide pitch variation). DECAY 0.4. Fast trigger (8th notes at 120 BPM). A dense, rhythmically complex insect storm.

---

## Basic setup — sound in 60 seconds

1. Add SwarmCore to your patch.
2. Wait 2–5 seconds for the ACTIVE LED to illuminate (bank loading).
3. Patch a clock or manual gate → TRIG IN.
4. Patch OUT L and OUT R → your audio interface.
5. MODE in Specimen. SPECIMEN at noon (mid-bank sample). DECAY 0.6. PITCH 0.
6. Trigger the clock. You hear a single insect call per trigger.
7. Press MODE button to switch to Swarm. DENSITY 0.7, SCATTER 0.2, DETUNE 0.3. Trigger again — a scattered chorus of 5–6 voices.

---

## How-tos

### Browsing specimens live

- Map SPECIMEN to MIDI CC 14 on a knob or jog wheel.
- Use relative CC encoding (1–63 = CW, 65–127 = CCW) for step-by-step browsing.
- Trigger constantly at a moderate rate while sweeping SPECIMEN to hear each insect sequentially.
- Note your preferred specimens: SPECIMEN 0 = first alphabetically, 1.0 = last.

### Rhythmic insect counterpoint

- Two SwarmCore instances. Instance A: Specimen mode, DECAY 0.2, a cicada sample, trigger on beats 1 and 3.
- Instance B: Swarm mode, DENSITY 0.5, SCATTER 0.3, an orthoptera sample, trigger on beats 2 and 4.
- Route A OUT → Choke CH1, B OUT → Choke CH3. The fixed panning separates the families spatially.
- Use different PITCH values (A: 0 st, B: +5 st) for register separation.

### V/OCT pitched playback

- Patch HarmonicPressure VOCT OUT (4 channels) → SwarmCore V/OCT IN.
- Specimen mode, DECAY 0.75, PITCH 0.
- Polyphonic triggers (one per channel) play the specimen at each harmonic partial pitch simultaneously.
- The insect sample becomes a pitched timbral texture — its spectral character reshapes at each transposition.

### Scatter as spatial motion

- Swarm mode, DENSITY 1.0 (8 voices), SCATTER modulated by Drift SMOOTH.
- Drift RATE 0.2 Hz, WANDER 0.5. SCATTER ATT +0.5.
- SCATTER value moves between 0.05 and 0.6 over slow cycles.
- At low SCATTER: tight, phase-coherent swarm. At high SCATTER: diffuse, evolving spatial cloud.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| COLLECTIVE (collective-refusal) | Swarm mode at DENSITY 1.0 — 8 insect voices as collective organism |
| SPACE (static-witness) | Specimen mode, single cicada at low trigger rate — a witness presence in an outdoor acoustic space |
| CARE (mutual-aid) | Low DENSITY Swarm + high DECAY — slow, overlapping insect calls, supportive ambient texture |
| PRESSURE (rent-pressure) | Fast trigger rate + high DENSITY + high DETUNE — dense, accumulating insect pressure |
| DAMAGE (managed-collapse) | PITCH extreme (±24 st) — the insect call shifted into damage register, no longer recognisable |
| GRID (work-clock) | Trigger from Pulse step grid — insects locked to percussive work-clock rhythm |

---

## Known pairings

| Module | Role |
| --- | --- |
| Pulse | Step gate → TRIG IN for rhythmic insect patterns |
| Drift | SMOOTH → SCATTER or DENSITY CV for slow swarm modulation; GATE → TRIG IN for stochastic triggers |
| HarmonicPressure | V/OCT → V/OCT IN for pitched harmonic texture from insect samples |
| Choke | OUT L/R into CH inputs for panning and level control alongside other sources |
| CollapseSat | OUT → IN for saturation of the insect texture (EVEN mode adds warmth) |
| WallConductor | OUT → CH input as an environmental texture layer beneath the drone wall |
