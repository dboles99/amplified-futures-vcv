# Send — 12HP

2×2 cross-send feedback routing matrix. Routes signals A and B into each other with configurable send levels, plus an internal C feedback bus with a one-sample delay for safe self-oscillation. Polyphonic.

---

## Signal flow

```text
A IN ──►── A→B SEND ────────────────────────────────► B OUT
       └── A→C DEPTH ──► [C bus, 1-sample delay] ──►──┐
                                                        │
B IN ──►── B→A RETURN ──────────────────────────────►──┤── A OUT
       └──────────────────── C→A RETURN ◄──────────────┘

OUT A = tanh(A IN + B→A × B IN + C→A × cBus)
OUT B = A→B × A IN
C bus = A→C × A IN  [delayed one sample]
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| A→B SEND | 0 | 0–1 | 0.5 | Level of A routed into B output |
| B→A RETURN | 1 | 0–1 | 0.5 | Level of B routed back into A output |
| A→C DEPTH | 2 | 0–1 | 0 | Feed depth into the internal C feedback bus |
| C→A RETURN | 3 | 0–1 | 0 | Amount of C bus fed back into A output |
| A→B ATTEN | 4 | −1 to +1 | 0 | Attenuverter for A→B CV |
| B→A ATTEN | 5 | −1 to +1 | 0 | Attenuverter for B→A CV |
| A→C ATTEN | 6 | −1 to +1 | 0 | Attenuverter for A→C CV |
| C→A ATTEN | 7 | −1 to +1 | 0 | Attenuverter for C→A CV |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| A IN | Input | Audio / poly | Primary input |
| B IN | Input | Audio / poly | Secondary input |
| A→B CV | Input | CV | CV for A→B SEND (scaled by A→B ATTEN) |
| B→A CV | Input | CV | CV for B→A RETURN |
| A→C CV | Input | CV | CV for A→C DEPTH |
| C→A CV | Input | CV | CV for C→A RETURN |
| V/OCT IN | Input | CV | Pass-through |
| A OUT | Output | Audio / poly | A + B→A return + C→A return, soft-clipped |
| B OUT | Output | Audio / poly | B + A→B send |
| V/OCT THRU | Output | CV | Pass-through |

The C bus is internal — there is no separate C OUT jack. If you need to tap the C bus externally, use FeedbackGovernor on A OUT.

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | A→B SEND | CC 14 |
| 1 | B→A RETURN | CC 15 |
| 2 | A→C DEPTH | CC 16 |
| 3 | C→A RETURN | CC 17 |

---

## Recommended configurations

**Drone Feedback Loop** — A→C DEPTH 0.4, C→A RETURN 0.3, A→B SEND 0, B→A RETURN 0. DroneClone OUT → A IN; A OUT → DroneClone RTN. The C bus creates a stable resonant loop.

**Cross-Bleed** — A→B SEND 0.25, B→A RETURN 0.25, C bus at 0. Two different signal sources blend into each other's outputs. Subtle timbral cross-contamination between two DroneClone instances.

**Resonant C-Bus** — A→C DEPTH 0.5, C→A RETURN 0.4, B→A RETURN 0. Push both C parameters to build resonance. The one-sample delay prevents DC runaway. Add FeedbackGovernor on A OUT for safety.

**Section Bleed** — A→B SEND 0.15, B→A RETURN 0.15. Two WallConductor section outputs bleed into each other at low levels. Creates spatial coherence without full summing.

---

## Basic setup — sound in 60 seconds

1. Add Send to your patch.
2. Patch DroneClone OUT → Send A IN.
3. Patch Send A OUT → DroneClone RTN.
4. Set A→C DEPTH to 0.3, C→A RETURN to 0.2. All other sends at 0.
5. Play a sustained note. The A output now includes a one-sample delayed version of itself.
6. Slowly raise C→A RETURN. At around 0.5 you will notice resonance buildup.
7. Keep A→C below 0.6 to avoid runaway.

---

## How-tos

### Governed C-bus feedback

- Patch Send A OUT → FeedbackGovernor SEND.
- Patch FeedbackGovernor RETURN → Send B IN.
- Set B→A RETURN to 0.4. The C bus output is filtered and attenuated by the Governor before returning.
- FeedbackGovernor TONE CV from Drift for slow feedback tone animation.

### Two-oscillator cross-contamination

- DroneCore A OUT → Send A IN. DroneCore B OUT → Send B IN.
- A→B SEND 0.2, B→A RETURN 0.2, C bus at 0.
- Each oscillator's character bleeds into the other's output — effective for beating interference between two different DETUNE settings.

### Stutter routing

- Patch Pulse GATE → a VCA or attenuverter controlling A→B SEND CV.
- At each percussion trigger, the A→B send briefly opens, injecting the wall signal into the B channel for a rhythmic bleed.

### Build from silence

- Set all sends to 0 at patch time. Map A→C DEPTH to MIDI CC 16 and C→A RETURN to CC 17.
- Perform by slowly raising both simultaneously. The feedback loop builds organically from a dry signal.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| COLLECTIVE (collective-refusal) | Cross-bleed (A→B + B→A both active) — signals lose individual identity, become collective |
| FAILURE (managed-collapse) | C→A RETURN pushed toward 0.6–0.7 — the loop accumulates toward instability |
| SPACE (static-witness) | C bus at low values, A→B minimal — spatial routing without collapse, signals remain discrete but connected |
| DAMAGE (managed-collapse) | Sudden C→A spike via CV — brief resonance injection as a structural event |

---

## Known pairings

| Module | Role |
| --- | --- |
| DroneClone | Primary feedback loop via A IN → RTN |
| FeedbackGovernor | Govern the C-bus output — filter and decay before return |
| WallConductor | Section bleed between two conductor channels |
| Drift | A→C DEPTH or C→A RETURN CV for slow modulation of feedback depth |
