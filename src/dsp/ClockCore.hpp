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
        phase = 0.0;
        // Clear the sag and reseed, or a reset taken while BROWNOUT is up
        // would inherit the previous run's dip and its RNG position. Reset
        // must mean reset.
        sag = 0.0;
        rngState = 0x1234567u;
        pulseCount = 0;
        firstPulsePending = true;
    }

    // Advance one pulse from an external clock edge, ignoring the internal
    // phase entirely. Divisions stay coherent because they count pulses
    // rather than elapsed time.
    Ticks tickExternal() {
        Ticks t;
        emit(t);
        return t;
    }

    // One sample. `bpm` is quarter-notes per minute.
    Ticks process(float bpm, float swing, float brownout, bool running) {
        Ticks t;
        if (!running) return t;

        // Phase is accumulated in double. In float, an increment such as
        // 2/48000 is not exactly representable and the rounding error makes
        // the period alternate between 24000 and 23999 samples - a clock that
        // is audibly steady but not actually steady. A test caught this.
        const double hz = static_cast<double>(clampf(bpm, 1.f, 1000.f)) / 60.0;

        // Fire immediately on the first sample after reset, so a run always
        // begins on a downbeat rather than one period later.
        if (firstPulsePending) {
            firstPulsePending = false;
            emit(t);
            return t;
        }

        // BROWNOUT: a mains-sag metaphor. The grid dips under load and
        // recovers, so this only ever SLOWS the clock and always returns to
        // nominal. It never drifts freely and never runs fast - a brownout
        // that ran fast would be a wobble, and would push the patch ahead of
        // the beat rather than dragging behind it.
        //
        // At exactly 0 the RNG is not consulted at all and the sag state is
        // forced to zero, so timing is bit-identical to a clean clock.
        // "Nearly steady" would defeat the purpose: nobody patches a clock
        // they cannot trust.
        double hzEff = hz;
        if (brownout > 0.f) {
            const double b = static_cast<double>(clampf(brownout, 0.f, 1.f));
            // On average about one sag every two seconds at full depth.
            const double pPerSample = b * 0.5 / static_cast<double>(sampleRate);
            if (randUnit() < pPerSample) sag = 1.0;
            // Exponential recovery, roughly a 250 ms time constant.
            sag *= std::exp(-1.0 / (0.25 * static_cast<double>(sampleRate)));
            if (sag < 1e-4) sag = 0.0;
            // At the deepest point the clock runs at 60 % of nominal.
            hzEff = hz * (1.0 - 0.4 * b * sag);
        } else {
            sag = 0.0;
        }

        phase += hzEff / static_cast<double>(sampleRate);

        // Swing: odd-numbered pulses arrive late by a fraction of the period
        // and even-numbered ones correspondingly early, so each PAIR still
        // spans exactly two beats. Without that symmetry a swung clock would
        // slowly drift away from the tempo it claims to run at.
        const double sw = static_cast<double>(clampf(swing, 0.f, 0.75f)) * 0.5;
        const double threshold = ((pulseCount % 2) == 1) ? (1.0 + sw) : (1.0 - sw);

        if (phase >= threshold) {
            phase -= threshold;
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

    // Deterministic xorshift32. std::rand would make the tests depend on
    // global process state, and Rack forbids allocation in process() anyway.
    double randUnit() {
        rngState ^= rngState << 13;
        rngState ^= rngState >> 17;
        rngState ^= rngState << 5;
        return static_cast<double>(rngState & 0xFFFFFFu) / 16777216.0;
    }

    float sampleRate = 44100.f;
    double phase = 0.0;
    double sag = 0.0;
    uint32_t pulseCount = 0;
    uint32_t rngState = 0x1234567u;
    bool firstPulsePending = true;
};
