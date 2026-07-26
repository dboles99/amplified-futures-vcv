# Mass Driver — 32 HP (AF-01)

![Mass Driver in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/MassDriver.png)

Sixteen-channel no-wave signal mixer, and the flagship of the AF-01 series. Wall Conductor's ideas at four times the width: DENSITY sweeps sixteen channels rather than four, PRESSURE and MASS separate drive from output level, and three output pairs let you take the same mix saturated, clean, or in mono. Channels 1–8 run down the left spine, 9–16 down the right, with the master controls in the centre.

If Wall Conductor is a performance surface, Mass Driver is a whole desk.

---

## Sound in 60 seconds

1. Add Mass Driver. Patch sources into any of **CH1**–**CH16**, and **OUT L/R** to your interface.
2. DENSITY starts at 100% and MASS at 75%, so everything patched is already audible.
3. Sweep **DENSITY** down. Channels drop away in order, sixteen to one.
4. Raise **PRESSURE** towards 100%. Drive climbs from 1× to 4× into the saturator.
5. Take **AUX L/R** to a second pair of inputs. That is the same mix *before* PRESSURE — clean, for parallel processing.
6. Hit **COLLAPSE**. Everything ducks at once.

---

## Signal flow

~~~text
CH1–16 IN ──► × GAIN (0–2×) × densityGain × (1 − muted)
                     │
                     ├─ pan: linear spread across 16 channels, −WIDTH … +WIDTH
                     └─ constant-power sum into mixL / mixR
                     │
        mix += previous output × FEEDBACK   [one-sample bus, capped 0.92]
                     │
        ┌────────────┴─────────────┬──────────────────┐
        │                          │                  │
   pre-PRESSURE               mono sum          × PRESSURE drive (1–4×)
   × MASS                     × MASS                  │ tanh
        │                          │             × collapseEnv × MASS
        ▼                          ▼                  ▼
    AUX L / AUX R              SUM (mono)       OUT L / OUT R

V/OCT IN ─────────────────────────────────────────────► V/OCT THRU
~~~

---

## Controls

![Mass Driver panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/MassDriver.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| GAIN ×16 | 0–2× | 0.75 | Per-channel level. CH1–8 on the left spine, CH9–16 on the right |
| MUTE ×16 | button | off | Per-channel mute |
| DENSITY | 0–100% | 100% | Sweeps 0 to 16 channels in |
| PRESSURE | 0–100% | 25% | Drive into tanh, 1× to 4× |
| WIDTH | 0–100% | 80% | Stereo spread. 0 is mono, 100% is full L/R |
| MASS | 0–100% | 75% | Master output level, applied to all three output pairs |
| FEEDBACK | 0–100% | 0% | One-sample recirculation, internally capped at 0.92 |
| COLLAPSE | button + gate | — | Instant duck to silence |

DENSITY, PRESSURE, WIDTH, MASS and FEEDBACK each have an attenuverter (−1 to +1) and a CV input. The per-channel GAIN knobs and MUTE buttons do not.

MASS and PRESSURE are worth keeping distinct in your head: PRESSURE decides how hard the signal hits the saturator, MASS decides how loud the result is. Raising PRESSURE and lowering MASS gives more distortion at the same volume.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| CH1–16 | Input | Channel audio |
| DENSITY / PRESSURE / WIDTH / MASS / FEEDBACK CV | Input | One per master knob, each via its attenuverter |
| COLLAPSE | Input | Gate — collapses, same as holding the button |
| OUT L / OUT R | Output | Main stereo, post-PRESSURE, post-collapse, post-MASS |
| AUX L / AUX R | Output | The same stereo mix **before** PRESSURE, with MASS applied |
| SUM | Output | Mono sum, with MASS applied |
| V/OCT IN → THRU | In / Out | Pass-through |

---

## Patch recipes

**Parallel saturation.** OUT L/R and AUX L/R both to your interface, or to two channels of another mixer. Blend the driven and clean versions of the identical mix — parallel distortion without a second module.

**Sixteen-voice assembly.** Sixteen sources, DENSITY at 0, swept up across a long take. Each quarter-turn brings in four more channels. The most direct use of the module.

**Mono check.** SUM to a single input. WIDTH can be pushed hard on OUT L/R while SUM confirms nothing collapses badly when summed.

**Feedback mass.** FEEDBACK 60%, PRESSURE 60%, MASS 40%. The recirculation accumulates and saturates each pass; the internal 0.92 cap keeps it from running away, and MASS keeps the level sane.

**Structural collapse.** [[Drift]] GATE → COLLAPSE at a slow RATE. Occasional, unpredictable silences across the whole sixteen-channel wall.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneClone]] | Several instances across the channel spines |
| [[String-Mass-Core]] | OUT → a channel; DENSITY then acts over the whole mass |
| [[Collapse-Saturator]] | AUX L/R → IN for a second, differently-shaped distortion path |
| [[Feedback-Governor]] | OUT → Feedback Governor → a spare channel, a loop with tone control |
| [[Drift]] | SMOOTH → DENSITY or MASS CV for slow structural movement |
| [[Wall-Conductor]] | Use Wall Conductor for sub-sections and Mass Driver as the master |

---

## See also

[[Wall-Conductor]] · [[Choke]] · [[Collapse-Saturator]] · [[Design-System]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/MassDriver.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/MassDriver.md)
