// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Standalone tests for SitarStringCore. No Rack, no VCV SDK - plain g++.
//
// These exist because Sitar Grid shipped silent. Every audio output read
// exactly 0.0 V in every build the module has ever had, and nothing caught
// it: the 14 original modules keep their DSP inline, so none of it was ever
// driven outside a plugin host. The first assertion below is the one that
// would have failed on day one.
#include "../src/dsp/SitarStringCore.hpp"
#include <cmath>
#include <cstdio>

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

static const float SR   = 44100.f;
static const float C4   = 261.6256f;

// Peak absolute output over `seconds`, and the time the string last rose
// above -60 dB relative to that peak.
struct Ring { float peak; float decayTime; bool finite; };

static Ring ringOut(SitarStringCore::PluckedString& s,
                    float freq, float damping, float seconds) {
    const int n = (int)(SR * seconds);
    Ring r{0.f, 0.f, true};
    float* env = new float[n];
    for (int i = 0; i < n; i++) {
        const float v = s.tick(freq, damping, SR);
        if (!std::isfinite(v)) r.finite = false;
        env[i] = std::fabs(v);
        if (env[i] > r.peak) r.peak = env[i];
    }
    if (r.peak > 0.f) {
        const float thr = r.peak / 1000.f;
        for (int i = n - 1; i >= 0; i--)
            if (env[i] > thr) { r.decayTime = (i + 1) / SR; break; }
    }
    delete[] env;
    return r;
}

int main() {
    using namespace SitarStringCore;

    // ── The regression that matters ──────────────────────────
    // A plucked string must produce audible output. Reading the excitation
    // back is the entire contract; everything below is detail.
    {
        Rng rng(1);
        PluckedString s;
        s.pluck(C4, 1.f, 0.6f, SR, rng);
        const Ring r = ringOut(s, C4, 0.3f, 2.f);
        check(r.peak > 0.f, "a plucked string is not silent");
        check(r.peak > 0.1f, "a plucked string reaches an audible level");
        check(r.peak <= 1.5f, "a plucked string does not exceed unity by much");
        check(r.finite, "a plucked string stays finite");
    }

    // The burst has to arrive at the output promptly, not a lap later. One
    // delay period at C4 is ~3.8 ms; allow two.
    {
        Rng rng(2);
        PluckedString s;
        s.pluck(C4, 1.f, 0.6f, SR, rng);
        const int len = delayLength(C4, SR, PluckedString::MAX_LEN);
        float early = 0.f;
        for (int i = 0; i < len * 2; i++) {
            const float v = std::fabs(s.tick(C4, 0.3f, SR));
            if (v > early) early = v;
        }
        check(early > 0.1f, "the burst reaches the output within two periods");
    }

    // ── Ring time ────────────────────────────────────────────
    // The loop gain is applied once per traversal, so damping reads as ring
    // time. If it were applied per sample the string would die in under 2 ms
    // and this would fail.
    {
        Rng rng(3);
        PluckedString s;
        s.pluck(C4, 1.f, 0.6f, SR, rng);
        const Ring r = ringOut(s, C4, 0.3f, 3.f);
        check(r.decayTime > 0.05f, "at default damping the string rings past 50 ms");
        check(r.decayTime < 2.f,   "at default damping the string is not endless");
    }

    // More damping must ring shorter, or the control does nothing.
    {
        Rng a(4), b(4);
        PluckedString lo, hi;
        lo.pluck(C4, 1.f, 0.6f, SR, a);
        hi.pluck(C4, 1.f, 0.6f, SR, b);
        const Ring rl = ringOut(lo, C4, 0.1f, 6.f);
        const Ring rh = ringOut(hi, C4, 0.9f, 6.f);
        check(rl.decayTime > rh.decayTime, "less damping rings longer than more");
    }

    // ── Silence and reset ────────────────────────────────────
    {
        PluckedString s;
        const Ring r = ringOut(s, C4, 0.3f, 0.25f);
        check(r.peak == 0.f, "an unplucked string outputs exactly zero");
    }
    {
        Rng rng(5);
        PluckedString s;
        s.pluck(C4, 1.f, 0.6f, SR, rng);
        ringOut(s, C4, 0.1f, 0.05f);
        s.reset();
        const Ring r = ringOut(s, C4, 0.1f, 0.05f);
        check(r.peak == 0.f, "reset silences a ringing string");
    }

    // ── Chikari drone string ─────────────────────────────────
    {
        Rng rng(6);
        DroneString d;
        d.pluck(C4 * 2.f, 0.8f, SR, rng);
        float peak = 0.f;
        bool finite = true;
        for (int i = 0; i < (int)(SR * 0.5f); i++) {
            const float v = d.tick(C4 * 2.f, SR);
            if (!std::isfinite(v)) finite = false;
            if (std::fabs(v) > peak) peak = std::fabs(v);
        }
        check(peak > 0.1f, "a plucked chikari string is not silent");
        check(finite, "the chikari string stays finite");
    }
    {
        DroneString d;
        float peak = 0.f;
        for (int i = 0; i < (int)(SR * 0.1f); i++)
            peak = std::fmax(peak, std::fabs(d.tick(C4 * 2.f, SR)));
        check(peak == 0.f, "an unplucked chikari string outputs exactly zero");
    }

    // ── Sympathetic bank ─────────────────────────────────────
    // Fed only by the main string, so a silent main string silenced these
    // too. They must ring from input and must not run away.
    {
        SympatheticBank bank;
        float peak = 0.f;
        for (int i = 0; i < (int)(SR * 0.2f); i++)
            peak = std::fmax(peak, std::fabs(bank.tick(0, C4, 0.9f, 0.f, SR)));
        check(peak == 0.f, "an undriven sympathetic string outputs exactly zero");
    }
    {
        SympatheticBank bank;
        float peak = 0.f;
        bool finite = true;
        // 50 ms of drive, then let it ring for 450 ms at a high feedback.
        for (int i = 0; i < (int)(SR * 0.5f); i++) {
            const float in = (i < (int)(SR * 0.05f))
                           ? std::sin(2.f * 3.14159265f * C4 * i / SR) : 0.f;
            const float v = bank.tick(0, C4, 0.99f, in, SR);
            if (!std::isfinite(v)) finite = false;
            peak = std::fmax(peak, std::fabs(v));
        }
        check(peak > 0.f,   "a driven sympathetic string rings");
        check(peak < 10.f,  "a sympathetic string at 0.99 feedback stays bounded");
        check(finite,       "a sympathetic string stays finite");
    }

    // ── Pitch tracking ───────────────────────────────────────
    // A higher note is a shorter delay line, or the pitch knob is decorative.
    {
        check(delayLength(C4 * 2.f, SR, PluckedString::MAX_LEN)
            < delayLength(C4, SR, PluckedString::MAX_LEN),
              "an octave up is a shorter delay line");
        check(delayLength(1.f, SR, PluckedString::MAX_LEN)
              <= PluckedString::MAX_LEN - 1,
              "a sub-audio pitch is clamped inside the buffer");
    }

    std::printf("\n%s\n", g_failures ? "FAILURES" : "all sitar string checks passed");
    return g_failures ? 1 : 0;
}
