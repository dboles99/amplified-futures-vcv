# Choke — 18 HP

![Choke in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/Choke.png)

Four-channel mixer built as an instrument rather than a utility. Each channel has GAIN, a TONE lowpass and a MUTE button, and the pan positions are fixed — hard left, left-of-centre, right-of-centre, hard right — so where a signal sits in the stereo field is decided by which jack you patch. The MAIN stage saturates, so pushing a channel's GAIN past unity drives the master rather than simply making it louder.

The fixed panning is deliberate. It removes a decision and forces you to think about signal placement as arrangement.

---

## Sound in 60 seconds

1. Add Choke. Patch a source into **CH1 IN**, and **L OUT** / **R OUT** to your interface.
2. Defaults are already musical: GAIN 0.75 (unity), TONE 0.7, MAIN 0.8.
3. You hear the source hard left — CH1's pan is fixed there.
4. Patch a second source into **CH4 IN**. It arrives hard right, and you have stereo width without a panner.
5. Push CH1 **GAIN** past 1.0. The MAIN tanh starts to saturate; the character changes rather than the level.
6. Hit **MUTE 1**. The channel silences but keeps its place in the field.

---

## Signal flow

~~~text
CH1 IN ──► GAIN ──► TONE (LP blend) ──► [MUTE] ──► pan L    ──►┐
CH2 IN ──► GAIN ──► TONE             ──► [MUTE] ──► pan L−C  ──►┤
CH3 IN ──► GAIN ──► TONE             ──► [MUTE] ──► pan R−C  ──►┤► MAIN ─► tanh ─► L OUT
CH4 IN ──► GAIN ──► TONE             ──► [MUTE] ──► pan R    ──►┘                  R OUT

Fixed pan positions: CH1 = full L, CH2 = 0.33, CH3 = 0.67, CH4 = full R
Polyphonic inputs are summed to mono per channel.
V/OCT IN ──────────────────────────────────────────────────► V/OCT THRU
~~~

---

## Controls

![Choke panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Choke.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| GAIN ×4 | 0–1.5× | 0.75 | Channel level. Unity sits at 0.75, so there is headroom above noon |
| TONE ×4 | 0–100% | 70% | Lowpass blend. 0 is dark, around 400 Hz; 100% is fully open |
| MUTE ×4 | button | off | Silences the channel and keeps its pan position |
| MAIN | 0–1.5× | 0.8 | Master level, followed by soft tanh saturation |

Each GAIN, each TONE and MAIN has an attenuverter (−1 to +1) and a CV input. The MUTE buttons do not.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| CH1–4 IN | Input | Per-channel audio. Polyphonic signals are summed to mono |
| GAIN 1–4 CV | Input | One per channel, via its attenuverter |
| TONE 1–4 CV | Input | One per channel, via its attenuverter |
| MAIN CV | Input | Master level, via its attenuverter |
| L OUT / R OUT | Output | Stereo master, after saturation |
| V/OCT IN → THRU | In / Out | Pass-through |

---

## Patch recipes

**Quad wall mix.** Four [[DroneClone]] or [[String-Mass-Core]] voices into CH1–4, all GAIN 0.75, TONE 0.7, MAIN 0.8. The fixed spread gives immediate stereo width with nothing patched to achieve it.

**Frequency stack.** CH1 sub layer (TONE 30%, GAIN 0.9), CH2 mid wall (TONE 70%, GAIN 0.75), CH3 high shimmer (TONE 100%, GAIN 0.5), CH4 percussion (TONE 80%, GAIN 1.0). TONE becomes a per-channel filter instead of a global EQ.

**Saturation pump.** One or two channels at GAIN 1.3–1.5, MAIN at 0.6. Those channels clip into the MAIN tanh while the overall level stays controlled — the behaviour of a driven channel on a cheap no-input mixer.

**Rhythmic mute grid.** [[Pulse]] driving gates into the MUTE CV inputs. Channels drop in and out on a grid while the drone layers underneath stay constant.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneClone]] | Multiple instances into CH1–4 for a spread wall |
| [[DroneCore]] | A stack of four, one per channel, with per-voice gain and muting |
| [[Pulse]] | Gates into MUTE CV for rhythmic gating |
| [[Drift]] | SMOOTH → a TONE CV for a slow per-channel filter sweep |
| [[Collapse-Saturator]] | L/R OUT → IN when the MAIN saturation is not enough |

---

## See also

[[Wall-Conductor]] · [[Mass-Driver]] · [[Pulse]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/Choke.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/Choke.md)
