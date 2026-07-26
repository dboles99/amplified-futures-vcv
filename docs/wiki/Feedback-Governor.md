# Feedback Governor — 12 HP

![Feedback Governor in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/FeedbackGovernor.png)

Feedback that behaves. Patch a signal into SEND, take RETURN back into your chain, and the module gives you a loop you can actually play: AMOUNT sets how much comes back, TONE filters what comes back, and DECAY makes each pass quieter than the last so the loop dies away instead of building. AMOUNT is hard-capped at 0.95, there is a DC blocker on the path, and the output hard-clips at ±10 V.

KILL zeroes the whole thing instantly, which is the control you will reach for on stage.

---

## Sound in 60 seconds

1. Put Feedback Governor in a loop: something's output → **SEND**, and **RETURN** back into that same something's input, or into [[Send]] IN B.
2. AMOUNT starts at 50%, TONE at 80%, DECAY at 0%.
3. Send a short sound through. It repeats, and with DECAY at zero it keeps repeating.
4. Raise **DECAY**. Each pass is now quieter — at 100% that is −24 dB per pass, so the tail dies in a few repeats.
5. Turn **TONE** down. The returns get darker each pass, the way real feedback in a room does.
6. Press **KILL**. Silence, immediately.

---

## Signal flow

~~~text
SEND IN ──► feedback path
              │
              ├─ × AMOUNT                    (0–95%, hard capped)
              ├─ × 2^(−4 × DECAY)            (at 100%: −24 dB per pass)
              ├─ 1-pole LP, TONE 100 Hz → 20 kHz
              ├─ DC blocker (5 Hz high-pass)
              └─ hard clip at ±10 V
              │
     KILL button / KILL gate ──► zero the path
              │
              ▼
          RETURN OUT ──► back into the chain

V/OCT IN ─────────────────────────────► V/OCT THRU
~~~

---

## Controls

![Feedback Governor panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/FeedbackGovernor.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| AMOUNT | 0–100% | 50% | Feedback level. Internally capped at 0.95 so unity is unreachable |
| TONE | 0–100% | 80% | Lowpass on the feedback path: 100 Hz at zero, 20 kHz at full |
| DECAY | 0–100% | 0% | Per-pass attenuation. 0% sustains; 100% is −24 dB every pass |
| KILL | button + gate | — | Zeroes the feedback path instantly |

AMOUNT, TONE and DECAY each have an attenuverter (−1 to +1) and a CV input. KILL has a gate input.

The interaction to learn is AMOUNT against DECAY. AMOUNT sets how loud the loop is; DECAY sets whether it is a sustain or a tail. High AMOUNT with zero DECAY is a drone that never ends. High AMOUNT with high DECAY is a big, short reverb-like swell.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| SEND | Input | Signal in from the chain |
| AMOUNT / TONE / DECAY CV | Input | One per knob, each via its attenuverter |
| KILL | Input | Gate — zeroes the path, same as the button |
| RETURN | Output | Processed signal, back into the chain |
| V/OCT IN → THRU | In / Out | Pass-through |

---

## Patch recipes

**The standard loop.** [[Wall-Conductor]] L OUT → SEND, RETURN → [[Send]] IN B with B→A up. Feedback around the whole wall, with tone and decay under your hands.

**String wall recirculation.** [[DroneClone]] OUT → SEND, RETURN → DroneClone RTN. AMOUNT below 40% before pushing TENSION, or the wall and the loop compound each other.

**Dark tail.** AMOUNT 70%, TONE 25%, DECAY 60%. Each pass darker and quieter — an echo that decays into low rumble rather than hiss.

**Endless drone.** AMOUNT 90%, DECAY 0%, TONE 80%. Feed it once and it sustains. KILL is how you end it.

**Rhythmic kill.** [[Pulse]] gate → KILL. The feedback tail is chopped on the grid while the source underneath continues.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Send]] | RETURN → IN B, so the B→A path carries the loop |
| [[DroneClone]] | RETURN → RTN, the module's dedicated feedback return |
| [[Wall-Conductor]] | L/R OUT → SEND for feedback around the whole mix |
| [[Mass-Driver]] | OUT → SEND, RETURN → a spare channel |
| [[Pulse]] | Gate → KILL for rhythmic interruption of the tail |

---

## See also

[[Send]] · [[Collapse-Saturator]] · [[Wall-Conductor]] · [[DroneClone]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/FeedbackGovernor.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/FeedbackGovernor.md)
