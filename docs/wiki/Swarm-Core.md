# Swarm Core — 18 HP

![Swarm Core in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/SwarmCore.png)

Bio-acoustic sample engine playing recorded insects. The bank is InsectSet32 — cicadas and orthoptera, field recordings rather than synthesis — and Swarm Core plays them in one of two ways. Specimen mode fires a single pitched voice per trigger. Swarm mode fires up to eight, scattered in time, detuned against each other and spread across a fixed stereo field, which turns individual calls into a chorus.

It is the only module in the plugin that reads files from disk, and that has two consequences worth knowing before you patch it.

With nothing patched into **TRIG**, DENSITY drives an internal event clock from 0.5 to 30 Hz, so the module sounds on its own; patch TRIG and it takes over. **SPECIMEN** now has an attenuverter and a CV input, so which insect plays can be sequenced. **MODE** latches rather than needing to be held.

---

## Sound in 60 seconds

1. Add Swarm Core. **Wait 2–5 seconds.** The bank loads on a background thread and the module outputs silence until it finishes — the ACTIVE LED lights when it is ready.
2. Patch a clock into **TRIG**, and **OUT L**/**OUT R** to your interface.
3. Defaults are SPECIMEN 0, DENSITY 50%, SCATTER 10%, DETUNE 20%, DECAY 50%, in Specimen mode. Each trigger plays one insect call.
4. Sweep **SPECIMEN** while triggering. You are scrolling through the bank, one recording at a time.
5. Press **MODE**. The SWARM LED lights and each trigger now fires several voices — raise **DENSITY** for more of them, **SCATTER** to spread them in time, **DETUNE** to spread them in pitch.

> **If the sample folder is missing**, Swarm Core falls back to a noise burst rather than going silent. If it sounds like noise instead of insects, the bank did not load — check that `res/insects/insectset32/` came with your install.

---

## Signal flow

~~~text
[background thread] load up to 64 WAVs from res/insects/insectset32/
                    ACTIVE LED dark, output silent, 2–5 s
                    bank ready → safe to read from the audio thread

TRIG IN (rising edge) ──► fire
V/OCT IN ──────────────► pitch offset in semitones, added to PITCH

  SPECIMEN mode:
      one voice · sample = bank[SPECIMEN × bankSize]
      speed = 2^(semitones/12) × (bankRate / hostRate)
      × envelope shaped by DECAY

  SWARM mode:
      up to 8 voices, fired in succession with SCATTER delays
      voice i: speed × 2^(DETUNE × offset[i] / 12)
      fixed pan spread: −1, −0.71, −0.33, 0, 0, 0.33, 0.71, 1
      active voices = ceil(DENSITY × 8)
                    │
        mix × 1/√8 ─┴──► OUT L / OUT R
        envelope sum ───► CV OUT (0–10 V)
~~~

---

## Controls

![Swarm Core panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/SwarmCore.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| SPECIMEN | 0–100% | 0 | Sample select, mapped linearly across the loaded bank |
| PITCH | −24 to +24 st | 0 | Playback pitch offset. 0 plays at the original rate |
| DENSITY | 0–100% | 50% | Swarm voice count, 1 to 8. Ignored in Specimen mode |
| SCATTER | 0–100% | 10% | Timing spread between swarm voices. 0 fires them together |
| DETUNE | 0–100% | 20% | Per-voice pitch spread in Swarm mode |
| DECAY | 0–100% | 50% | Playback envelope. Low fades fast; high plays the full sample |
| MODE | button | Specimen | Toggles Specimen / Swarm. The SWARM LED shows the state |

PITCH, DENSITY, SCATTER and DETUNE have attenuverters and CV inputs. **DECAY has a CV input but no attenuverter** — its CV adds directly to the knob. SPECIMEN and MODE have neither.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| TRIG | Input | Rising edge fires. In Swarm mode it fires every active voice, with scatter |
| V/OCT | Input | Pitch offset, added to the PITCH knob |
| DENSITY / SCATTER / DETUNE CV | Input | Via their attenuverters |
| DECAY CV | Input | Added directly to the knob — no attenuverter on this one |
| OUT L / OUT R | Output | Stereo, 1/√8 normalised |
| CV OUT | Output | Summed voice envelope, 0–10 V |

CV OUT is the useful oddity here: it is an envelope follower on Swarm Core's own output, so the swarm can drive other modules in time with itself.

---

## The sample bank

- **InsectSet32**, [Zenodo 7072196](https://zenodo.org/record/7072196), CC-BY 4.0.
- Cicadidae (cicadas) and orthoptera (crickets, grasshoppers, katydids).
- Up to 64 WAV files loaded from `res/insects/insectset32/`, recursively, sorted.
- 16-bit or float32 PCM, mono or stereo. Stereo files are read as mono from the left channel.
- Maximum 5 seconds loaded per sample.

---

## Patch recipes

**Single cicada.** Specimen mode, DECAY 70%, triggered from [[Drift]] GATE at RATE 15%. One call at irregular intervals — presence rather than rhythm.

**Cricket chorus.** Swarm mode, DENSITY 80%, SCATTER 15%, DETUNE 30%, DECAY 60%, on a steady clock. A dense field that still reads as individual insects.

**Storm swarm.** Swarm mode, DENSITY 100%, SCATTER 40%, DETUNE 60%, DECAY 40%, triggered on eighth notes. Eight voices, widely scattered — no longer identifiable as insects.

**Pitched texture.** Specimen mode, [[Harmonic-Pressure]] V/OCT OUT → V/OCT IN, DECAY 80%. The same recording plays at several harmonic partials at once and stops sounding like a field recording.

**Self-modulation.** CV OUT → SCATTER CV or [[Collapse-Saturator]] SIDECHAIN. The swarm's own envelope drives what happens to it.

**Locked to the grid.** [[Pulse]] and Swarm Core on one clock. Percussion and insects land together; PITCH at +12 st separates them in register.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Pulse]] | Share a clock, or take Pulse's grid as the trigger source |
| [[Drift]] | GATE → TRIG for stochastic calls; SMOOTH → SCATTER for drifting spread |
| [[Harmonic-Pressure]] | V/OCT → V/OCT IN for pitched harmonic texture |
| [[Choke]] | OUT L/R into channels, alongside the drone layers |
| [[Collapse-Saturator]] | OUT → IN, or CV OUT → SIDECHAIN for self-driven distortion |
| [[Wall-Conductor]] | OUT → a channel as an environmental layer under the wall |

---

## See also

[[Pulse]] · [[Drift]] · [[Sitar-Grid]] · [[Playbooks]]

Sample bank: [InsectSet32](https://zenodo.org/record/7072196) — CC-BY 4.0.

**Full parameter spec:** [`docs/modules/SwarmCore.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/SwarmCore.md)
