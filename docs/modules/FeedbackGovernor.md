# Feedback Governor — 12HP

Controlled feedback send/return. Takes a signal, filters it, attenuates it per-pass, and returns it — creating a governed feedback loop that decays rather than runaway. KILL button/gate zeros the path instantly. DC blocker and ±10V safety limiter on every pass.

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

V/OCT IN ──────────────────────────────────────────► V/OCT THRU
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| AMOUNT | 0–1 | Output level of the return signal |
| TONE | 0–1 | LP filter cutoff: 0 = 100Hz (very dark), 1 = 20kHz (full open) |
| DECAY | 0–1 | Per-pass attenuation: 0 = no decay (stable loop), 1 = 1/16× per pass (fast fade) |
| KILL | Button | Zeros the feedback path immediately. Hold for sustained silence |

AMOUNT and TONE have attenuverter + CV. DECAY has attenuverter + CV.

---

## DECAY formula

`effectiveAmount = AMOUNT × 0.5^(DECAY × 4)`

| DECAY | Multiplier |
|---|---|
| 0.0 | 1.0× (no attenuation) |
| 0.25 | 0.5× (−6dB per pass) |
| 0.5 | 0.25× (−12dB per pass) |
| 1.0 | 0.0625× (−24dB per pass) |

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

- **← DroneClone** OUT → SEND; RETURN → RTN — primary use case
- **← Send** C OUT → SEND for C-bus feedback governing
- **→ DroneClone** RTN, or back to WallConductor CH input
- **← Drift** SMOOTH → TONE CV for feedback tone animation
- **← Pulse** GATE → KILL GATE for rhythmic feedback chopping
