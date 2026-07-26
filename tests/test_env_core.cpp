// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Standalone tests for EnvCore. No Rack, no VCV SDK - plain g++.
#include "../src/dsp/EnvCore.hpp"
#include <cmath>
#include <cstdio>
#include <vector>

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

// Run to completion (or `maxSamples`), returning every output sample.
static std::vector<float> run(EnvCore& e, float sr, float a, float d, float curve,
                              int maxSamples) {
    std::vector<float> out;
    out.reserve(maxSamples);
    for (int i = 0; i < maxSamples; ++i) {
        bool eoc = false;
        out.push_back(e.process(a, d, curve, &eoc));
        if (eoc) break;
    }
    return out;
}

int main() {
    const float SR = 48000.f;

    // Idle output is silence. An envelope that idles above zero would hold a
    // VCA permanently open.
    {
        EnvCore e; e.setSampleRate(SR); e.reset();
        bool nonZero = false;
        for (int i = 0; i < 4800; ++i) {
            bool eoc = false;
            if (e.process(0.01f, 0.1f, 0.f, &eoc) != 0.f) nonZero = true;
        }
        check(!nonZero, "idle envelope outputs exactly zero");
        check(!e.isRunning(), "idle envelope reports not running");
    }

    // A full cycle rises to 1, comes back to 0, and reports EOC once.
    {
        EnvCore e; e.setSampleRate(SR); e.reset();
        check(e.trigger(0.f), "trigger with misfire=0 always fires");
        std::vector<float> v = run(e, SR, 0.01f, 0.05f, 0.f, static_cast<int>(SR));
        float peak = 0.f;
        for (size_t i = 0; i < v.size(); ++i) if (v[i] > peak) peak = v[i];
        check(peak > 0.99f, "envelope reaches full level");
        check(v.back() <= 1e-6f, "envelope returns to zero");
        check(!e.isRunning(), "envelope is idle after EOC");
    }

    // Attack and decay times must actually control duration.
    {
        EnvCore a1; a1.setSampleRate(SR); a1.reset(); a1.trigger(0.f);
        std::vector<float> shortRun = run(a1, SR, 0.01f, 0.05f, 0.f, static_cast<int>(SR * 4));

        EnvCore a2; a2.setSampleRate(SR); a2.reset(); a2.trigger(0.f);
        std::vector<float> longRun = run(a2, SR, 0.01f, 0.40f, 0.f, static_cast<int>(SR * 4));

        check(longRun.size() > shortRun.size() * 3,
              "a longer decay produces a proportionally longer envelope");

        // 10 ms attack + 50 ms decay = 60 ms = 2880 samples at 48 kHz.
        const int expected = static_cast<int>(SR * 0.06f);
        check(std::abs(static_cast<int>(shortRun.size()) - expected) < expected / 10,
              "total duration matches attack + decay within 10%");
    }

    // Curve must change the shape without changing the endpoints, or a curve
    // knob would double as a level knob.
    {
        auto midpoint = [&](float curve) {
            EnvCore e; e.setSampleRate(SR); e.reset(); e.trigger(0.f);
            std::vector<float> v = run(e, SR, 0.2f, 0.2f, curve, static_cast<int>(SR * 2));
            return v.empty() ? 0.f : v[v.size() / 4];   // partway up the attack
        };
        const float lin = midpoint(0.f);
        const float log = midpoint(-1.f);
        const float exp = midpoint(1.f);
        check(log > lin, "negative curve rises faster than linear");
        check(exp < lin, "positive curve rises slower than linear");

        // Endpoints unchanged.
        for (float c : {-1.f, 0.f, 1.f}) {
            EnvCore e; e.setSampleRate(SR); e.reset(); e.trigger(0.f);
            std::vector<float> v = run(e, SR, 0.01f, 0.05f, c, static_cast<int>(SR));
            float peak = 0.f;
            for (size_t i = 0; i < v.size(); ++i) if (v[i] > peak) peak = v[i];
            check(peak > 0.99f && v.back() <= 1e-6f, "curve preserves 0 and 1 endpoints");
        }
    }

    // Output must never leave 0..1, whatever the settings.
    {
        EnvCore e; e.setSampleRate(SR); e.reset();
        bool inRange = true, finite = true;
        for (int rep = 0; rep < 20; ++rep) {
            e.trigger(0.f);
            for (int i = 0; i < 2000; ++i) {
                bool eoc = false;
                const float v = e.process(0.0001f, 0.0001f, 1.f, &eoc);
                if (v < 0.f || v > 1.f) inRange = false;
                if (!std::isfinite(v)) finite = false;
            }
        }
        check(inRange, "output stays within 0..1 at extreme settings");
        check(finite, "output is always finite");
    }

    // MISFIRE at 0 must be EXACTLY reliable - the same promise BROWNOUT makes
    // on the clock. A utility you cannot trust at default goes unused.
    {
        EnvCore e; e.setSampleRate(SR); e.reset();
        int fired = 0;
        for (int i = 0; i < 2000; ++i) if (e.trigger(0.f)) ++fired;
        check(fired == 2000, "misfire=0 never drops a trigger");
    }

    // Turned up, it must actually drop some.
    {
        EnvCore e; e.setSampleRate(SR); e.reset();
        int fired = 0;
        for (int i = 0; i < 2000; ++i) if (e.trigger(1.f)) ++fired;
        check(fired < 2000, "misfire=1 drops triggers");
        check(fired > 0, "misfire=1 does not drop everything");
    }

    // Reset must make a run reproducible even with misfire up, or the same
    // patch would behave differently on reload. ClockCore had exactly this bug.
    {
        EnvCore a, b;
        a.setSampleRate(SR); b.setSampleRate(SR);
        a.reset(); b.reset();
        std::vector<int> fa, fb;
        for (int i = 0; i < 500; ++i) fa.push_back(a.trigger(0.6f) ? 1 : 0);
        for (int i = 0; i < 500; ++i) fb.push_back(b.trigger(0.6f) ? 1 : 0);
        check(fa == fb, "reset is reproducible WITH misfire up");
    }

    std::printf(g_failures ? "\nFAILED (%d)\n" : "\nAll EnvCore tests passed\n",
                g_failures);
    return g_failures ? 1 : 0;
}
