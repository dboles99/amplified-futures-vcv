// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Standalone tests for VcaCore. No Rack, no VCV SDK - plain g++.
#include "../src/dsp/VcaCore.hpp"
#include <cmath>
#include <cstdio>
#include <initializer_list>   // range-for over braced lists needs this in C++11

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

int main() {
    using namespace VcaCore;

    // Both curves must agree at the endpoints, or flipping LIN/EXP would
    // jump the level rather than only changing the travel between.
    {
        check(gainCurve(0.f, false) == 0.f, "linear: 0 maps to 0");
        check(gainCurve(1.f, false) == 1.f, "linear: 1 maps to 1");
        check(gainCurve(0.f, true) == 0.f,  "exponential: 0 maps to 0");
        check(std::fabs(gainCurve(1.f, true) - 1.f) < 1e-6f,
              "exponential: 1 maps to 1 exactly");
    }

    // Exponential must sit below linear in between - that is what makes it
    // exponential rather than a differently-named linear.
    {
        bool below = true, monotonic = true;
        float prev = -1.f;
        for (int i = 1; i < 100; ++i) {
            const float x = i / 100.f;
            const float e = gainCurve(x, true);
            if (e >= gainCurve(x, false)) below = false;
            if (e < prev) monotonic = false;
            prev = e;
        }
        check(below, "exponential sits below linear across the sweep");
        check(monotonic, "exponential is monotonic");
    }

    {
        bool clamped = true;
        for (float x : {-5.f, -0.001f, 1.001f, 5.f})
            for (bool e : {false, true}) {
                const float g = gainCurve(x, e);
                if (g < 0.f || g > 1.f) clamped = false;
            }
        check(clamped, "gain is clamped to 0..1 outside the input range");
    }

    // PRESSURE at 0 must be BIT-transparent. A mixer that colours the signal
    // with its character knob down cannot be used as a clean mixer.
    {
        bool identical = true;
        for (int i = -200; i <= 200; ++i) {
            const float x = i / 20.f;
            if (saturate(x, 0.f) != x) identical = false;
        }
        check(identical, "pressure=0 returns the input bit-identically");
    }

    // Odd symmetry: an asymmetric curve would inject DC into the sum.
    {
        bool odd = true;
        for (float x : {0.1f, 0.5f, 1.f, 3.f})
            for (float p : {0.25f, 0.5f, 1.f})
                if (std::fabs(saturate(x, p) + saturate(-x, p)) > 1e-6f) odd = false;
        check(odd, "saturation is odd-symmetric, so it adds no DC");
    }

    // Full scale in stays full scale out: the knob adds harmonics rather than
    // quietly turning the signal down.
    {
        bool unity = true;
        for (float p : {0.1f, 0.5f, 1.f})
            if (std::fabs(saturate(1.f, p) - 1.f) > 1e-5f) unity = false;
        check(unity, "full-scale input maps to full-scale output at any pressure");
    }

    {
        bool finite = true, bounded = true;
        for (float x : {-1e6f, -10.f, 0.f, 10.f, 1e6f})
            for (float p : {0.f, 0.5f, 1.f}) {
                const float y = saturate(x, p);
                if (!std::isfinite(y)) finite = false;
                // Beyond unity input the output may exceed 1 only when
                // pressure is 0, where it is a pass-through by definition.
                if (p > 0.f && std::fabs(y) > 1.001f) bounded = false;
            }
        check(finite, "saturation output is always finite");
        check(bounded, "saturation bounds the output once engaged");
    }

    {
        check(saturate(0.f, 0.f) == 0.f, "silence stays silent, pressure 0");
        check(saturate(0.f, 1.f) == 0.f, "silence stays silent, pressure 1");
    }

    // More pressure must mean more gain on a small signal, or the knob does
    // nothing audible where it matters most.
    {
        const float x = 0.1f;
        check(saturate(x, 1.f) > saturate(x, 0.25f),
              "more pressure lifts a small signal further");
    }

    std::printf(g_failures ? "\nFAILED (%d)\n" : "\nAll VcaCore tests passed\n",
                g_failures);
    return g_failures ? 1 : 0;
}
