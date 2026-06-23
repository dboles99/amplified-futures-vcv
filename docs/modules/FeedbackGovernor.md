# Feedback Governor — 12HP

Controlled feedback send/return. Takes a signal, filters it, attenuates it per-pass, and returns it — creating a governed feedback loop that decays rather than running away. KILL button/gate zeros the path instantly. DC blocker (5 Hz HP) and ±10 V safety limiter on every pass.

---

## Signal flow

```text
SEND IN ──► TONE LP filter (100 Hz–20 kHz, 1-pole) ──► AMOUNT level
                │                                           │
                │      effAmount = AMOUNT × 0.5^(DECAY×4) ─┘
                │
            DC blocker (5 Hz HP) ──► safety clamp ±10 V ──► RETURN OUT

KILL BTN / KILL IN ──► instantly zeros lpState + hpState, bypasses output

V/OCT IN ──────────────────────────────────────────────────► V/OCT THRU
```

DECAY formula: `effectiveAmount = AMOUNT × 0.5^(DECAY × 4)`

| DECAY | Multiplier per pass |
| --- | --- |
| 0.0 | 1.0× (no attenuation — stable self-sustaining loop) |
| 0.25 | 0.5× (−6 dB per pass) |
| 0.5 | 0.25× (−12 dB per pass) |
| 0.75 | 0.125× (−18 dB per pass) |
| 1.0 | 0.0625× (−24 dB per pass — tail dies within 4–5 passes) |

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| AMOUNT | 0 | 0–1 | 0.5 | Output level. Hard-capped at 0.95 — prevents infinite runaway |
| AMOUNT ATTEN | 1 | −1 to +1 | 0 | Attenuverter for AMOUNT CV |
| TONE | 2 | 0–1 | 0.8 | LP cutoff: 0 = 100 Hz (very dark), 1 = 20 kHz (full open) |
| TONE ATTEN | 3 | −1 to +1 | 0 | Attenuverter for TONE CV |
| DECAY | 4 | 0–1 | 0 | Per-pass attenuation: 0 = no decay, 1 = −24 dB per pass |
| DECAY ATTEN | 5 | −1 to +1 | 0 | Attenuverter for DECAY CV |
| KILL | 6 | Button | — | Zeros the feedback path immediately |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| SEND IN | Input | Audio | Signal entering the feedback path |
| AMOUNT CV | Input | CV | CV for AMOUNT (scaled by AMOUNT ATTEN) |
| TONE CV | Input | CV | CV for TONE |
| DECAY CV | Input | CV | CV for DECAY |
| KILL IN | Input | Gate | High = path zeroed instantly |
| V/OCT IN | Input | CV | Pass-through |
| RETURN OUT | Output | Audio | Processed feedback return |
| V/OCT THRU | Output | CV | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | AMOUNT | CC 14 |
| 2 | TONE | CC 15 |
| 4 | DECAY | CC 16 |

KILL button (index 6) can be mapped to a momentary CC (value ≥ 64 triggers). Useful for a physical kill switch on a pad controller.

---

## Recommended configurations

**Governed Drone Loop** — AMOUNT 0.3, TONE 0.6, DECAY 0.2. DroneClone OUT → SEND; RETURN → DroneClone RTN. Feedback gradually darkens and fades — a self-dying resonance trail.

**Self-Sustaining Tone** — AMOUNT 0.5, TONE 0.7, DECAY 0 (no decay). The loop sustains indefinitely. TONE CV from Drift for slow filter animation. KILL on downbeats for punctuation.

**Dark Reverb Tail** — AMOUNT 0.35, TONE 0.2, DECAY 0.8 (fast fade). The feedback tail dies within 4–5 passes (≈ 20–50 ms depending on loop length). Behaviour approximates a short dark reverb without the complexity.

**Bright Feedback Arc** — AMOUNT 0.4, TONE 1.0, DECAY 0.15. Full bandwidth feedback — no darkening. Drift SMOOTH → TONE CV (TONE ATTEN −0.6) for inverse brightening: the feedback brightens as SMOOTH rises.

---

## Basic setup — sound in 60 seconds

1. Add FeedbackGovernor to your patch.
2. Patch DroneClone OUT → FeedbackGovernor SEND IN.
3. Patch FeedbackGovernor RETURN OUT → DroneClone RTN IN.
4. Set AMOUNT to 0.3, TONE to 0.7, DECAY to 0.2.
5. You hear a governed feedback return — the loop darkens and attenuates each pass.
6. Raise AMOUNT to 0.5 for a stronger loop. Watch for level buildup.
7. Press KILL to instantly clear the loop. Use as a performance event.

---

## How-tos

### Tone-animated feedback tail

- Patch Drift SMOOTH → TONE CV. TONE ATTEN +0.5.
- Set AMOUNT 0.35, DECAY 0.25, TONE 0.5.
- The feedback path slowly brightens and darkens as Drift wanders.
- Set Drift RATE 0.12, WANDER 0.45. The tail never has the same character twice.

### KILL as rhythmic event

- Patch Pulse GATE OUT → FeedbackGovernor KILL IN.
- Programme Pulse with a pattern: step 1 and step 9 active.
- On every downbeat the feedback path clears — a sharp transient followed by the loop rebuilding.
- Keep DECAY at 0 (no attenuation) so the loop builds back to the same level between kills.

### Post-collapse feedback tail

- Chain: WallConductor OUT → CollapseSat → FeedbackGovernor SEND.
- FeedbackGovernor RETURN → Send B IN; Send B→A RETURN → WallConductor CH3.
- When CollapseSat COLLAPSE fires, the burst enters the governor and forms a decaying tail.
- FeedbackGovernor KILL IN tied to CollapseSat COLLAPSE IN so the tail clears before the next collapse.

### Parallel wet/dry blend

- Patch the dry source to both WallConductor CH1 (direct) and FeedbackGovernor SEND (parallel).
- Patch FeedbackGovernor RETURN → WallConductor CH2 (lower GAIN, different pan position).
- AMOUNT 0.25, TONE 0.5, DECAY 0.4. The feedback return adds a blurred ghosting effect at lower level.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| FAILURE (managed-collapse) | DECAY 0 + AMOUNT near 0.95 — the loop sustains at maximum, approaching instability |
| CARE (mutual-aid) | TONE low + DECAY 0.6 — the feedback warms and fades gently, supportive rather than dominating |
| DAMAGE (managed-collapse) | KILL on downbeats — rhythmic erasure of the accumulated feedback |
| SPACE (static-witness) | Long DECAY-free loops at low AMOUNT — sound persisting in space, witnessing itself |
| PRESSURE (rent-pressure) | AMOUNT high + DECAY 0 — the loop never fades, accumulates without relief |

---

## Known pairings

| Module | Role |
| --- | --- |
| DroneClone | OUT → SEND; RETURN → RTN — primary use case |
| Send | C OUT → SEND for C-bus feedback governing |
| CollapseSat | OUT → SEND for post-collapse tail governing |
| WallConductor | RETURN → CH input for blended feedback return |
| Drift | SMOOTH → TONE CV for feedback tone animation |
| Pulse | GATE → KILL IN for rhythmic feedback chopping |
