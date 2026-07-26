# Send — 12 HP

![Send in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/Send.png)

A 2×2 cross-send routing matrix with a hidden third path. A and B cross into each other, and a separate internal bus — C — takes a tap from A and returns it to A one sample later. That one-sample delay is what lets Send self-oscillate without running away: the loop cannot close within a single sample, so the feedback stays bounded and the output stage soft-clips whatever survives.

There is no C jack. C exists only inside the module; the A→C and C→A knobs are its send and return.

---

## Sound in 60 seconds

1. Add Send. Patch any audio into **IN A** and **OUT A** to your interface.
2. Signal passes straight through — A→B and B→A start at 50%, A→C and C→A at zero.
3. Turn **A→C** and **C→A** up together, slowly. A resonant tail builds as A feeds itself.
4. Keep going. It thickens and eventually self-oscillates, but it will not blow up — the output is soft-clipped.
5. Patch a second source into **IN B** and turn **B→A** up. Now two sources share one feedback loop.

---

## Signal flow

~~~text
IN A ──┬──────────────────────────────────────────► ┐
       │                                             │
       ├─ × A→B ─────────────────────────────────► OUT B
       │                                             │
       └─ × A→C ──► cBus  [one-sample delay]         │
                      │                              │
                      └─ × C→A ────────────────────► ┤
                                                     │
IN B ──── × B→A ───────────────────────────────────► ┤
                                                     │
                                          tanh ──► OUT A

V/OCT IN ──────────────────────────────────────► V/OCT THRU
~~~

`OUT A = tanh(IN A + B→A × IN B + C→A × cBus)` · `OUT B = A→B × IN A` · `cBus = A→C × IN A`

---

## Controls

![Send panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Send.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| A→B | 0–100% | 50% | How much of A is sent to OUT B |
| B→A | 0–100% | 50% | How much of B returns into OUT A |
| A→C | 0–100% | 0% | Send from A into the internal C bus — the feedback depth |
| C→A | 0–100% | 0% | Return from the C bus into A — the feedback amount |

Every knob has an attenuverter (−1 to +1) and a CV input.

A→C and C→A multiply, so the loop gain is roughly their product. Both at 50% is a gentle tail; both at 80% is on the edge of oscillation. Raising one while the other is at zero does nothing at all.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| IN A | Input | Main signal, polyphonic. Sets the channel count |
| IN B | Input | Second signal, polyphonic |
| A→B / B→A / A→C / C→A CV | Input | One per knob, each via its attenuverter |
| OUT A | Output | Main output — A, plus the B return, plus the C return, soft-clipped |
| OUT B | Output | The A→B send. Not soft-clipped |
| V/OCT IN → THRU | In / Out | Pass-through |

---

## Patch recipes

**Safe self-oscillation.** Nothing in IN A. A→C 90%, C→A 90%. Send oscillates on its own noise floor and the tanh keeps it in range — a drone source with no oscillator.

**Effects loop with feedback.** OUT B → an effect → IN B, with A→B 70% and B→A 50%. The effect returns into A, and A→C/C→A add recirculation on top of whatever the effect does.

**Cross-feedback between two walls.** Two [[DroneClone]] instances, one into IN A and one into IN B. B→A at 40%. Each wall colours the other without either being a straightforward send.

**Controlled tail.** OUT A → [[Feedback-Governor]] → IN B, with B→A moderate. Feedback Governor's TONE and DECAY shape what comes back, giving a filtered tail rather than a raw one.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneClone]] | RTN bus for cross-feedback between two instances |
| [[Feedback-Governor]] | OUT A → Feedback Governor → IN B for a shaped loop |
| [[Collapse-Saturator]] | OUT B → Collapse Saturator → IN B; distortion inside the loop |
| [[Wall-Conductor]] | Insert Send between a source and the wall for pre-mix recirculation |

---

## See also

[[Feedback-Governor]] · [[DroneClone]] · [[Wall-Conductor]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/Send.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/Send.md)
