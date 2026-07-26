# Feedback Governor — 12HP

![Feedback Governor panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/FeedbackGovernor.png)

Controlled feedback send/return. Takes a signal, filters it, attenuates it per-pass, and returns it — creating a governed feedback loop that decays rather than runs away. KILL button/gate zeros the path instantly. DC blocker and ±10V safety limiter on every pass.

---

## Signal flow

```
SEND IN ──► TONE (1-pole LP, 100Hz–20kHz) ──► DECAY attenuation ──► RETURN OUT
                │                                     │
AMOUNT ────────►│ level control                       │ per-pass: effAmount = AMOUNT × 0.5^(DECAY×4)
                │                                     │
KILL BTN/IN ──► │ zeros lpState + hpState, bypass ───┘
                │
DC blocker ────► hpState tracking (5Hz HP)
Safety limiter ► clamp(−10V, +10V)
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| AMOUNT | 0–1 | Output level of the return signal |
| TONE | 0–1 | LP filter cutoff: 0 = 100Hz (very dark), 1 = 20kHz (full open) |
| DECAY | 0–1 | Per-pass attenuation: 0 = no decay (stable loop), 1 = 1/16× per pass (fast fade) |
| KILL | Button | Zeros the feedback path immediately |

AMOUNT and TONE have attenuverter + CV. DECAY has attenuverter + CV.

---

## DECAY formula

`effectiveAmount = AMOUNT × 0.5^(DECAY × 4)`

| DECAY | Multiplier | dB per pass | Character |
|---|---|---|---|
| 0.0 | 1.000× | 0 dB | No attenuation — stable self-sustaining loop |
| 0.25 | 0.500× | −6 dB | Halves per pass — fades over ~5 passes |
| 0.5 | 0.250× | −12 dB | Fades quickly — reverb-like tail |
| 0.75 | 0.125× | −18 dB | Short tail |
| 1.0 | 0.0625× | −24 dB | Very fast fade — almost just delay |

At DECAY 0, AMOUNT controls loop level — above 1.0 effective level (impossible with AMOUNT ≤ 1) the loop would grow; the safety limiter prevents this.

---

## Ports

| Port | Type | Notes |
|---|---|---|
| SEND IN | Input | Signal entering the feedback path |
| GATE IN | Input | KILL gate — high = path zeroed |
| RETURN OUT | Output | Processed feedback return |
| CV (×3) | Input | CV for AMOUNT, TONE, DECAY |
| V/OCT IN | Input | Pass-through |
| V/OCT THRU | Output | Pass-through |

---

## Patch tips

- **Basic loop**: DroneClone OUT → SEND; RETURN → DroneClone RTN. Set DECAY 0.2, TONE 0.6, AMOUNT 0.3. The feedback gradually darkens and fades.
- **KILL as performance event**: patch to a manual button or Pulse GATE out for sudden feedback silences.
- **TONE automation**: Drift SMOOTH → TONE CV. The feedback tail slowly darkens and brightens as the loop cycles.
- **DECAY → 0 (no decay) + AMOUNT 0.2**: stable self-sustaining feedback loop. Add TONE CV for filter animation.
- **DECAY → 0.8**: feedback tail dies within 4–5 passes — useful as a reverb-like tail without true reverb character.
- Always patch **KILL → COLLAPSE IN** on CollapseSat when using heavy feedback — prevents DC buildup on collapse events.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneClone]] | OUT → SEND; RETURN → RTN — primary use case |
| [[Send]] | C OUT → SEND for C-bus feedback governing |
| [[Wall-Conductor]] | RETURN → CH input |
| [[Drift]] | SMOOTH → TONE CV for feedback tone animation |
| [[Pulse]] | GATE → KILL GATE for rhythmic feedback chopping |

---

## See also

[[Send]] · [[DroneClone]] · [[Collapse-Saturator]] · [[Playbooks]]
