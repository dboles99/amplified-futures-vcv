// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Standalone tests for ParamSlew. No Rack, no VCV SDK - plain g++.
#include "../src/dsp/ParamSlew.hpp"
#include <cmath>
#include <cstdio>

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

static const float SR = 48000.f;

int main() {
    using ParamSlew::Smoother;

    // The first value must land immediately. A smoother that ramps up from
    // zero on the first sample turns every patch load into a fade-in.
    {
        Smoother s;
        s.configure(5.f, SR);
        check(s.process(0.75f) == 0.75f, "first call jumps to the target");
    }

    // A step must not be passed through. That step is the click.
    {
        Smoother s;
        s.configure(5.f, SR);
        s.process(0.f);
        const float first = s.process(1.f);
        check(first > 0.f, "a step starts moving immediately");
        check(first < 0.25f, "a step is not passed through in one sample");
    }

    // It must actually arrive, and roughly when the time constant says.
    {
        Smoother s;
        s.configure(5.f, SR);
        s.process(0.f);
        int n = 0;
        while (s.value() < 0.632f && n < (int)SR) { s.process(1.f); n++; }
        const float ms = 1000.f * n / SR;
        check(ms > 3.5f && ms < 7.f, "reaches 63% at about the configured 5 ms");

        for (int i = 0; i < (int)SR; i++) s.process(1.f);
        check(std::fabs(s.value() - 1.f) < 1e-4f, "settles on the target");
    }

    // Monotonic for a monotonic input - no overshoot to ring on a gain.
    {
        Smoother s;
        s.configure(5.f, SR);
        s.process(0.f);
        float prev = 0.f;
        bool monotonic = true;
        for (int i = 0; i < 2000; i++) {
            const float v = s.process(1.f);
            if (v < prev - 1e-7f) monotonic = false;
            prev = v;
        }
        check(monotonic, "no overshoot on a rising step");
    }

    // Faster setting must actually be faster.
    {
        Smoother fast, slow;
        fast.configure(1.f, SR);
        slow.configure(20.f, SR);
        fast.process(0.f);
        slow.process(0.f);
        for (int i = 0; i < 100; i++) { fast.process(1.f); slow.process(1.f); }
        check(fast.value() > slow.value(), "1 ms setting outruns 20 ms");
    }

    // reset() must return it to jumping, not to zero-and-ramp.
    {
        Smoother s;
        s.configure(5.f, SR);
        s.process(1.f);
        s.reset();
        check(s.process(0.3f) == 0.3f, "reset makes the next value jump again");
    }

    // Stays finite and in range when driven hard.
    {
        Smoother s;
        s.configure(2.f, SR);
        bool finite = true, bounded = true;
        for (int i = 0; i < 20000; i++) {
            const float t = (i % 2) ? 1.f : 0.f;   // alternating every sample
            const float v = s.process(t);
            if (!std::isfinite(v)) finite = false;
            if (v < -0.001f || v > 1.001f) bounded = false;
        }
        check(finite, "stays finite under an alternating target");
        check(bounded, "stays within the target range");
    }

    // A sample rate change must not leave the old coefficient in place.
    {
        Smoother a, b;
        a.configure(5.f, 44100.f);
        b.configure(5.f, 96000.f);
        a.process(0.f);
        b.process(0.f);
        for (int i = 0; i < 100; i++) { a.process(1.f); b.process(1.f); }
        check(a.value() > b.value(),
              "the same time in ms takes more samples at a higher rate");
    }

    {
        ParamSlew::Bank<4> bank;
        bank.configure(5.f, SR);
        check(bank.process(0, 0.5f) == 0.5f && bank.process(3, 0.25f) == 0.25f,
              "bank smoothers are independent");
    }

    std::printf("\n%s\n", g_failures ? "FAILURES" : "all param slew checks passed");
    return g_failures ? 1 : 0;
}
