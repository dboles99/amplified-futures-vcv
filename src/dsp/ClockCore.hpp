#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// ClockCore — timing logic for AF-02 Street Grid Clock.
//
// Deliberately has NO Rack dependency so it can be unit-tested by a
// standalone binary (see tests/test_clock_core.cpp). The Rack Module is a
// thin wrapper that reads params and calls process() once per sample.
//
// This mirrors the commercial DSP rule already in force across the
// workspace: all DSP in a host-independent core.
// ============================================================
#include <cmath>
#include <cstdint>

struct ClockCore {
    // What fired on this sample. All false on most samples.
    struct Ticks {
        bool clk;
        bool div2;
        bool div4;
        bool div8;
        bool reset;
        Ticks() : clk(false), div2(false), div4(false), div8(false), reset(false) {}
    };

    void setSampleRate(float sr) {
        sampleRate = (sr > 0.f) ? sr : 44100.f;
    }

    void reset() {
        phase = 0.f;
        pulseCount = 0;
        firstPulsePending = true;
    }

    // One sample. `bpm` is quarter-notes per minute.
    Ticks process(float bpm, float swing, float brownout, bool running) {
        (void)swing; (void)brownout;   // Tasks 2 and 3
        Ticks t;
        if (!running) return t;

        const float hz = clampf(bpm, 1.f, 1000.f) / 60.f;

        // Fire immediately on the first sample after reset, so a run always
        // begins on a downbeat rather than one period later.
        if (firstPulsePending) {
            firstPulsePending = false;
            emit(t);
            return t;
        }

        phase += hz / sampleRate;
        if (phase >= 1.f) {
            phase -= 1.f;
            emit(t);
        }
        return t;
    }

protected:
    static float clampf(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    // Emit the base clock and whichever divisions land on this pulse.
    // Counting from 0 means pulse 0 fires every division, so all outputs
    // agree on the downbeat.
    void emit(Ticks& t) {
        t.clk  = true;
        t.div2 = (pulseCount % 2) == 0;
        t.div4 = (pulseCount % 4) == 0;
        t.div8 = (pulseCount % 8) == 0;
        ++pulseCount;
    }

    float sampleRate = 44100.f;
    float phase = 0.f;
    uint32_t pulseCount = 0;
    bool firstPulsePending = true;
};
