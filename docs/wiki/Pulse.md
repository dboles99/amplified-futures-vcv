# Pulse — 12 HP

![Pulse in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/Pulse.png)

Sixteen-step noise percussion on a 4×4 grid. One voice, one sound, four knobs to shape it — white noise through a body filter, with a separate crack transient on the attack. There is no sample bank and no pitch: Pulse makes a single percussive noise event and lets you place sixteen of them in time. It exists to put a rhythmic edge against material that otherwise has none.

---

## Sound in 60 seconds

1. Add Pulse. Patch a clock into **TRG** and **OUT** to your interface.
2. Click some step buttons in the 4×4 grid. Lit steps fire; unlit ones pass.
3. You hear a noise hit on each lit step, with the default HIT 75%, DECAY 30%, METAL 20%, CRACK 40%.
4. Turn **DECAY** up. Hits stretch from a tick towards a wash — 8 ms at the bottom of the dial, half a second at the top.
5. Turn **METAL** up. The body filter opens from 80 Hz towards 360 Hz and raw noise takes over — the hit moves from a thud to a hiss.

---

## Signal flow

~~~text
TRG IN (clock) ──► step counter, advances 0…15 and wraps
                          │
                   step lit? ──► fire: env = 1, crackEnv = 1
                          │
   white noise ──┬─ 1-pole LP ──► body    (METAL shifts 360 → 80 Hz)
                 ├─ raw × METAL ────────► grit
                 └─ raw × crackEnv × CRACK ► 4 ms transient
                          │
                 × exponential env (DECAY 8–500 ms) × HIT
                          │
                    2× tanh ──► OUT
V/OCT IN ─────────────────────────────────────────► V/OCT THRU
~~~

---

## Controls

![Pulse panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Pulse.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| STEP ×16 | on / off | all off | 4×4 toggle grid. Green-red LED shows state and playhead |
| HIT | 0–100% | 75% | Output level of the hit, before the output saturator |
| DECAY | 0–100% | 30% | Envelope decay, 8 ms to 500 ms |
| METAL | 0–100% | 20% | Body filter and grit. Low is a dull thud at 360 Hz; high opens to 80 Hz and mixes in raw noise |
| CRACK | 0–100% | 40% | Level of the 4 ms transient burst on the attack |

HIT, DECAY, METAL and CRACK each have an attenuverter (−1 to +1) and a CV input. The step buttons do not.

METAL is worth understanding because it does two things at once: it lowers the body filter's corner *and* raises the amount of unfiltered noise in the mix. That is why it moves the sound from body to hiss rather than simply brightening it.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| TRG | Input | Clock. Each trigger advances one step; lit steps fire |
| HIT / DECAY / METAL / CRACK CV | Input | One per knob, each via its attenuverter |
| OUT | Output | Audio, soft-clipped |
| V/OCT IN → THRU | In / Out | Pass-through, so Pulse can sit inline in a pitch chain |

---

## Patch recipes

**Work clock.** Steps 1, 5, 9, 13 lit — four on the floor. DECAY 20%, METAL 10%, CRACK 60%. A hard, dry pulse under a drone.

**Broken grid.** Steps 1, 4, 7, 11, 14 lit. DECAY 45%, METAL 40%. The pattern is sixteen long but never lands where a bar expects it to.

**Noise wash.** Most steps lit, DECAY 90%, METAL 80%, HIT 50%. The hits overlap into continuous noise rather than reading as rhythm.

**Stochastic hits.** [[Drift]] GATE → TRG instead of a clock, Drift RATE 60%. The grid advances irregularly, so the same sixteen steps produce a different rhythm every pass.

**Gate source.** Pulse is also a rhythm generator for other modules — use its clock to drive [[Swarm-Core]] TRG, or gate [[Choke]] MUTE inputs, so percussion and texture share a grid.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Drift]] | GATE → TRG for irregular, non-repeating triggering |
| [[Choke]] | Pulse into a channel, or its clock into MUTE for rhythmic gating |
| [[Swarm-Core]] | Share a clock so insect calls land on the percussion grid |
| [[Collapse-Saturator]] | OUT → IN; saturation thickens the noise body considerably |
| [[Wall-Conductor]] | OUT → channel input as the rhythmic layer under the wall |

---

## See also

[[Drift]] · [[Swarm-Core]] · [[Choke]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/Pulse.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/Pulse.md)
