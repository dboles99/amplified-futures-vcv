# Sitar Grid — 42 HP

![Sitar Grid in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/SitarGrid.png)

Modal string-resonance sequencer, and the largest module in the plugin. Three independent sequencers — one for pitch, one for timbre, one for articulation — drive a Karplus-Strong string through a nonlinear jawari bridge, with an eight-voice sympathetic resonator bank and a chikari drone ringing behind it. On top sits a breakdown engine: a state machine that builds, accelerates, breaks into jhala, and lands.

The three sequencers run on their own lengths and, for the resonance brain, its own clock division. Set them to different lengths and the module stops repeating in any obvious period.

---

## Sound in 60 seconds

1. Add Sitar Grid. Patch a clock into **CLOCK** and **MAIN L**/**MAIN R** to your interface.
2. It arrives playing: RAGA is Yaman, the pitch sequence is eight steps, and JAWARI sits at 35%.
3. You hear a plucked string with buzz, sympathetics ringing underneath, and the chikari drone at 30%.
4. Turn **RAGA** through its six settings. The same sequence re-quantises into Bilawal, Bhairav, Bhairavi, Kafi and Khamaj.
5. Set **RES LEN** to 5 while **PITCH LEN** stays at 8. The timbre sequence now cycles against the pitch sequence, and the pattern takes forty steps to repeat.
6. Raise **BD INT** and send a gate into **BD GATE**. The breakdown engine builds, accelerates into jhala, and lands.

---

## Signal flow

~~~text
CLOCK ──┬─► PITCH brain  (8 steps, PITCH LEN, direction fwd/pend/rand)
        │        └─► quantise to RAGA scale, + ROOT, + Sa gravity
        │                                            │
        ├─► RES brain    (8 steps, RES LEN, own clock division)
        │        └─► timbral resonance per step ─────┤
        │                                            │
        └─► RIFF brain   (8 steps, RIFF LEN)         │
                 └─► articulation / ornament ────────┤
                                                     ▼
                                    Karplus-Strong main string
                                    (DAMPING · BRIGHTNESS · MEEND glide)
                                                     │
                                    Jawari nonlinear bridge
                                    (JAWARI · EDGE · CHAOS)
                                                     │
                        ┌────────────────────────────┼──────────────┐
                        ▼                            ▼              ▼
            8-voice sympathetic bank        chikari drone      main string
            (DECAY · SPREAD · FEEDBACK)     (CHIKARI)               │
                        │                            │              │
                     SYMP OUT                    DRONE OUT     MAIN L / R

BD GATE ──► breakdown state machine: IDLE → BUILD → ACCEL → JHALA → LAND
            (BD INT · BD ACCEL · BD LAND)
LOCK GATE ──► freeze the sequencers where they are
~~~

---

## Controls

![Sitar Grid panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/SitarGrid.png)

### The three sequencers

| Control | Range | Default | What it does |
|---|---|---|---|
| PITCH STEP ×8 | 0–100% | ramped | Eight scale-degree steps, quantised to the current raga |
| PITCH LEN | 1–8 steps | 8 | How many pitch steps play before wrapping |
| PITCH DIR | fwd / pend / rand | fwd | Forward, pendulum, or random step order |
| RES STEP ×8 | 0–100% | 50% | Eight timbral-resonance steps |
| RES LEN | 1–8 steps | 5 | Length of the resonance sequence |
| RES DIV | 1–4 | 2 | Clock division for the resonance brain only |
| RIFF STEP ×8 | 0–7 | 0 | Eight articulation steps |
| RIFF LEN | 1–8 steps | 8 | Length of the riff sequence |

### Tuning and phrase

| Control | Range | Default | What it does |
|---|---|---|---|
| ROOT | −4 to +4 V/oct | 0 | Transposes everything |
| RAGA | 0–5 | 1 (Yaman) | Bilawal, Yaman, Bhairav, Bhairavi, Kafi, Khamaj |
| PHRASE LEN | 1–32 steps | 8 | Phrase length for the breakdown engine to work against |
| SA GRAVITY | 0–100% | 60% | How strongly the sequence is pulled back to the tonic |

### String and bridge

| Control | Range | Default | What it does |
|---|---|---|---|
| DAMPING | 0–100% | 30% | Karplus-Strong damping — how fast the string dies |
| BRIGHTNESS | 0–100% | 60% | String brightness |
| MEEND | 0–100% | 20% | Glide between pitches, after the sitar bend |
| JAWARI | 0–100% | 35% | Buzz amount at the bridge |
| JAWARI EDGE | 0–100% | 50% | Brightness of the buzz |
| JAWARI CHAOS | 0–100% | 10% | Flutter and instability in the buzz |

### Sympathetics, drone, breakdown

| Control | Range | Default | What it does |
|---|---|---|---|
| SYMP DECAY | 0–100% | 70% | How long the eight sympathetic voices ring |
| SYMP SPREAD | 0–100% | 50% | Tuning spread across the sympathetic bank |
| SYMP FEEDBACK | 0–100% | 60% | Sympathetic recirculation |
| CHIKARI | 0–100% | 30% | Density of the chikari drone string |
| ORNAMENT | 0–100% | 30% | Ornament density on the main line |
| BD INT | 0–100% | 0% | Breakdown intensity — at zero the engine stays idle |
| BD ACCEL | 0–100% | 50% | How fast the breakdown accelerates |
| BD LAND | 0–100% | 70% | How hard the breakdown lands on sam |

ROOT and JAWARI have dedicated CV inputs.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| CLOCK | Input | Drives all three sequencers; RES divides it by RES DIV |
| RESET | Input | Returns every sequencer to step 1 |
| V/OCT | Input | Pitch input |
| ROOT CV | Input | Transposition |
| JAWARI CV | Input | Buzz amount |
| BD GATE | Input | Triggers the breakdown state machine |
| LOCK GATE | Input | Freezes the sequencers at their current step |
| MAIN L / MAIN R | Output | The main stereo voice |
| DRONE | Output | Chikari drone, separately |
| SYMP | Output | Sympathetic bank, separately |
| PITCH CV | Output | The quantised pitch, for driving other modules |
| GATE | Output | Note gate |
| RIFF TRIG | Output | Trigger from the riff brain |
| RES CV | Output | The resonance sequence as CV |

The separate DRONE and SYMP outputs matter: take them to their own channels on [[Choke]] or [[Mass-Driver]] and you can balance string, drone and sympathetics independently rather than accepting the internal mix.

---

## Patch recipes

**Three-against-eight.** PITCH LEN 8, RES LEN 5, RIFF LEN 3. The three brains realign only every 120 steps, so a short clock produces a long non-repeating line.

**Sitar Grid as a pitch source.** Ignore the audio outputs. PITCH CV and GATE into [[DroneClone]] or [[String-Mass-Core]] — the raga quantiser and phrase logic drive the wall instead of a string.

**Breakdown as arrangement.** BD INT 70%, BD ACCEL 60%, BD LAND 90%, with [[Drift]] GATE → BD GATE at a very low RATE. The piece breaks down into jhala at unpredictable intervals and lands hard each time.

**Separated mix.** MAIN L/R, DRONE and SYMP each to their own [[Choke]] channels. Bring the sympathetics up under a quiet passage and drop them for the breakdown.

**Locked drone.** LOCK GATE held high. The sequencers freeze, the string keeps sounding, and JAWARI CV becomes the only thing moving — a static drone with a live bridge.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Choke]] | MAIN, DRONE and SYMP to separate channels for an independent balance |
| [[Mass-Driver]] | The same, with sixteen channels to spare |
| [[String-Mass-Core]] | PITCH CV → V/OCT IN; the raga logic drives the mass |
| [[Drift]] | GATE → BD GATE for unpredictable breakdowns |
| [[Collapse-Saturator]] | MAIN L/R → IN; EVEN mode suits the jawari buzz |
| [[Pulse]] | Share a clock so percussion and string land together |

---

## See also

[[Music-Theory]] · [[String-Mass-Core]] · [[DroneClone]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/SitarGrid.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/SitarGrid.md)
