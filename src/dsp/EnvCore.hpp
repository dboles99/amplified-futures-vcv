#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// EnvCore — envelope logic for AF-04 Collapse EG.
//
// No Rack dependency, so it can be unit-tested by a standalone binary
// (tests/test_env_core.cpp), the same arrangement as ClockCore.
//
// Attack-Decay with a continuous curve control and an opt-in MISFIRE that
// makes a proportion of triggers truncate or fail outright. MISFIRE at 0
// is a textbook AD envelope, exactly — the character is opt-in, as with
// BROWNOUT on the clock.
// ============================================================
#include <cmath>
#include <cstdint>

struct EnvCore {
    enum Stage { IDLE, ATTACK, DECAY };

    void setSampleRate(float sr) {
        sampleRate = (sr > 0.f) ? sr : 44100.f;
    }

    void reset() {
        stage = IDLE;
        level = 0.f;
        rngState = 0x9E3779B9u;
        eocPending = false;
    }

    // Fire the envelope. `misfire` 0..1 is the probability the hit is spoiled:
    // either skipped entirely or truncated to a fraction of full level.
    // Returns true if the envelope actually started.
    bool trigger(float misfire) {
        if (misfire > 0.f) {
            const float m = clampf(misfire, 0.f, 1.f);
            // Half the misfire probability drops the hit; the other half lets
            // it through at reduced ceiling. A dropped hit alone would just
            // sound like a broken module; a weak hit sounds like a struggling
            // one, which is the intent.
            if (randUnit() < m * 0.5f) {
                return false;
            }
            if (randUnit() < m * 0.5f) {
                ceiling = 0.25f + 0.5f * randUnit();
            } else {
                ceiling = 1.f;
            }
        } else {
            ceiling = 1.f;
        }
        stage = ATTACK;
        return true;
    }

    // One sample. `attack` and `decay` are in seconds; `curve` is -1..+1,
    // negative logarithmic, 0 linear, positive exponential.
    // Writes true to `eocOut` on the sample the envelope finishes.
    float process(float attack, float decay, float curve, bool* eocOut) {
        if (eocOut) *eocOut = false;

        switch (stage) {
        case ATTACK: {
            const float secs = clampf(attack, 0.0005f, 8.f);
            level += 1.f / (secs * sampleRate);
            if (level >= ceiling) {
                level = ceiling;
                stage = DECAY;
            }
            break;
        }
        case DECAY: {
            const float secs = clampf(decay, 0.0005f, 16.f);
            level -= ceiling / (secs * sampleRate);
            if (level <= 0.f) {
                level = 0.f;
                stage = IDLE;
                if (eocOut) *eocOut = true;
            }
            break;
        }
        case IDLE:
        default:
            break;
        }

        return shape(level, curve);
    }

    bool isRunning() const { return stage != IDLE; }
    Stage getStage() const { return stage; }

protected:
    static float clampf(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    // Apply the curve to a 0..1 level. Linear at curve == 0 exactly, so the
    // centre detent is a true straight line rather than an approximation.
    static float shape(float x, float curve) {
        if (x <= 0.f) return 0.f;
        if (x >= 1.f) return 1.f;
        const float c = clampf(curve, -1.f, 1.f);
        if (c == 0.f) return x;
        // Exponent sweeps 1/3 (logarithmic, fast rise) to 3 (exponential).
        const float e = std::pow(3.f, c);
        return std::pow(x, e);
    }

    // Deterministic xorshift32: std::rand would make tests depend on global
    // process state, and Rack forbids allocation in process() anyway.
    float randUnit() {
        rngState ^= rngState << 13;
        rngState ^= rngState >> 17;
        rngState ^= rngState << 5;
        return static_cast<float>(rngState & 0xFFFFFFu) / 16777216.f;
    }

    float sampleRate = 44100.f;
    float level = 0.f;
    float ceiling = 1.f;
    Stage stage = IDLE;
    uint32_t rngState = 0x9E3779B9u;
    bool eocPending = false;
};
