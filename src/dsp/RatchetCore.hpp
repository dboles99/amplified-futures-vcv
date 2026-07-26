#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// RatchetCore — burst generator for AF-03 Ratchet.
//
// One trigger in, a burst of repeats out. No Rack dependency, so it can be
// unit-tested by a standalone binary (tests/test_ratchet_core.cpp), as with
// ClockCore, EnvCore and VcaCore.
//
// There is no RATE control, by design. The burst fills the interval between
// the two most recent input triggers, so feeding it a clock subdivides that
// clock and the module stays in time without a knob to set. The cost is that
// the very first trigger passes through alone: the module cannot know the
// tempo before it has seen two edges, and guessing one would put the first
// burst at the wrong speed.
//
// Repeat 0 always fires, whatever PROB says. A ratchet that can swallow the
// downbeat is not usable as an insert.
// ============================================================
#include <cmath>
#include <cstdint>

struct RatchetCore {
    enum { MAX_REPEATS = 8 };

    struct Out {
        bool trig;   // burst pulse, high for one pulse width
        bool end;    // fires once the burst window has closed
    };

    RatchetCore() { setSampleRate(44100.f); }

    void setSampleRate(float sr) {
        sampleRate   = (sr > 0.f) ? sr : 44100.f;
        pulseSamples = int(0.001f * sampleRate);        // 1 ms, Rack convention
        if (pulseSamples < 1) pulseSamples = 1;
        // A measured interval outside this range is a patching accident, not a
        // tempo. Clamping keeps the scheduler's arithmetic in a sane range.
        minPeriod = int(0.02f * sampleRate);
        maxPeriod = int(8.f * sampleRate);
        reset();
    }

    void reset() {
        sinceInput    = 0;
        sawInput      = false;
        havePeriod    = false;
        periodSamples = 0;
        burstActive   = false;
        nScheduled    = 0;
        nextIdx       = 0;
        burstClock    = 0;
        endAt         = 0;
        trigHigh      = 0;
        endHigh       = 0;
        rngState      = 0x1a2b3c4du;
    }

    /// Advance one sample.
    ///   inputEdge — a rising edge arrived on TRIG this sample
    ///   count     — 1..8 repeats, the first being the pass-through
    ///   spread    — -1 decelerating, 0 even, +1 accelerating
    ///   prob      — 0..1 chance each repeat after the first actually fires
    Out process(bool inputEdge, int count, float spread, float prob) {
        Out out;
        out.trig = false;
        out.end  = false;

        if (sinceInput < (1 << 29)) sinceInput++;
        if (burstActive) burstClock++;

        if (inputEdge) {
            if (sawInput) {
                int measured = sinceInput;
                if (measured < minPeriod) measured = minPeriod;
                if (measured > maxPeriod) measured = maxPeriod;
                periodSamples = measured;
                havePeriod    = true;
            }
            sawInput   = true;
            sinceInput = 0;

            // A new edge restarts the burst rather than queueing behind it -
            // that is what makes it a ratchet and not a delay.
            schedule(count, spread, prob);
            burstClock  = 0;
            nextIdx     = 1;             // index 0 fires below, now
            burstActive = true;
            trigHigh    = pulseSamples;
        }

        while (burstActive && nextIdx < nScheduled && burstClock >= times[nextIdx]) {
            if (fires[nextIdx]) trigHigh = pulseSamples;
            nextIdx++;
        }
        if (burstActive && burstClock >= endAt) {
            endHigh     = pulseSamples;
            burstActive = false;
        }

        if (trigHigh > 0) { out.trig = true; trigHigh--; }
        if (endHigh  > 0) { out.end  = true; endHigh--;  }
        return out;
    }

    bool  hasPeriod()        const { return havePeriod; }
    float periodSeconds()    const { return float(periodSamples) / sampleRate; }
    int   scheduledRepeats() const { return nScheduled; }
    int   repeatSample(int i) const { return times[i]; }

protected:
    /// Lay out the burst for one input edge.
    ///
    /// Even spacing is t_i = (i/n) * period. SPREAD warps that through
    /// t_i = (i/n)^p, with p = 3^-spread: p < 1 is concave, so the early gaps
    /// are wide and the later ones narrow - accelerating. p > 1 decelerates.
    /// p == 1 at spread 0 gives exactly even spacing.
    void schedule(int count, float spread, float prob) {
        int n = count;
        if (n < 1) n = 1;
        if (n > MAX_REPEATS) n = MAX_REPEATS;

        times[0] = 0;
        fires[0] = true;                 // the pass-through, never dropped
        nScheduled = 1;

        if (n > 1 && havePeriod) {
            const float p  = std::pow(3.f, -clampf(spread, -1.f, 1.f));
            const float pr = clampf(prob, 0.f, 1.f);
            // Finish inside the window so the burst never runs into the next
            // input edge, and keep pulses from merging into one long gate.
            const int cap = periodSamples - pulseSamples;
            for (int i = 1; i < n; i++) {
                const float u = float(i) / float(n);
                int t = int(std::pow(u, p) * float(periodSamples) + 0.5f);
                const int floorT = times[nScheduled - 1] + 2 * pulseSamples;
                if (t < floorT) t = floorT;
                // Out of room: drop the remaining repeats rather than stack
                // them on top of each other. A short period simply holds fewer.
                if (t > cap) break;
                times[nScheduled] = t;
                fires[nScheduled] = (randUnit() < pr);
                nScheduled++;
            }
        }
        endAt = times[nScheduled - 1] + pulseSamples;
    }

    static float clampf(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    /// xorshift32. Local state, so a test can reset() and get the same burst
    /// twice; no global RNG to make results depend on call order.
    float randUnit() {
        rngState ^= rngState << 13;
        rngState ^= rngState >> 17;
        rngState ^= rngState << 5;
        return float(rngState & 0xffffffu) / float(0x1000000u);
    }

    float sampleRate;
    int   pulseSamples, minPeriod, maxPeriod;

    int   sinceInput, periodSamples;
    bool  sawInput, havePeriod;

    bool  burstActive;
    int   nScheduled, nextIdx, burstClock, endAt;
    int   times[MAX_REPEATS];
    bool  fires[MAX_REPEATS];

    int   trigHigh, endHigh;
    uint32_t rngState;
};
