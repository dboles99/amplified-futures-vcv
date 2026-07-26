// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Standalone tests for BlocCore. No Rack, no VCV SDK - plain g++.
//
// These use == deliberately. Signal Bloc is the module you reach for when you
// want to know exactly what happened to a voltage, so "close enough" is the
// wrong bar.
#include "../src/dsp/BlocCore.hpp"
#include <cstdio>
#include <initializer_list>

static int g_failures = 0;

static void check(bool cond, const char* what) {
    if (!cond) { std::printf("  FAIL: %s\n", what); ++g_failures; }
    else       { std::printf("  ok:   %s\n", what); }
}

int main() {
    using namespace BlocCore;

    // Unity and zero offset must be bit-transparent across the whole range.
    {
        bool identical = true;
        for (int i = -1000; i <= 1000; ++i) {
            const float v = i / 100.f;      // -10 .. +10 V
            if (scaleOffset(v, 1.f, 0.f) != v) identical = false;
        }
        check(identical, "att=1, off=0 returns the input bit-identically");
    }

    // Attenuverting to zero must give exactly the offset, not the offset plus
    // a residue of the input.
    {
        bool exact = true;
        for (float in : {-10.f, -3.3f, 0.f, 1.f, 7.7f, 10.f})
            for (float off : {-5.f, -1.f, 0.f, 2.5f, 5.f})
                if (scaleOffset(in, 0.f, off) != off) exact = false;
        check(exact, "att=0 outputs exactly the offset, whatever the input");
    }

    // Inversion must be exact, or a signal and its inversion would not cancel.
    {
        bool cancels = true;
        for (int i = -500; i <= 500; ++i) {
            const float v = i / 50.f;
            if (sum3(scaleOffset(v, 1.f, 0.f), scaleOffset(v, -1.f, 0.f), 0.f) != 0.f)
                cancels = false;
        }
        check(cancels, "a signal and its inversion sum to exactly zero");
    }

    {
        bool linear = true;
        for (float in : {-8.f, -2.f, 0.f, 3.f, 6.f})
            for (float att : {-1.f, -0.5f, 0.f, 0.25f, 1.f})
                for (float off : {-5.f, 0.f, 5.f}) {
                    const float want = in * att + off;
                    if (want >= -12.f && want <= 12.f &&
                        scaleOffset(in, att, off) != want) linear = false;
                }
        check(linear, "scale and offset is exactly in*att+off below the rails");
    }

    // The adder must be a precision adder. No soft knee anywhere in range.
    {
        bool exact = true;
        for (float a : {-5.f, -1.5f, 0.f, 2.f, 4.f})
            for (float b : {-3.f, 0.f, 1.25f})
                for (float c : {-2.f, 0.f, 3.5f}) {
                    const float want = a + b + c;
                    if (want >= -12.f && want <= 12.f && sum3(a, b, c) != want)
                        exact = false;
                }
        check(exact, "the adder is exact everywhere below the rails");
    }
    {
        check(sum3(0.f, 0.f, 0.f) == 0.f, "an empty sum is exactly zero");
        check(sum3(5.f, -5.f, 0.f) == 0.f, "opposing inputs cancel exactly");
    }

    // Rack requires modules to stay within +/-12 V. That is the only
    // nonlinearity allowed here, and it must be a hard clamp, not a curve.
    {
        check(sum3(10.f, 10.f, 10.f) == 12.f,  "the adder clamps at +12 V");
        check(sum3(-10.f, -10.f, -10.f) == -12.f, "the adder clamps at -12 V");
        check(scaleOffset(10.f, 1.f, 5.f) == 12.f, "scale+offset clamps at +12 V");
        check(scaleOffset(-10.f, 1.f, -5.f) == -12.f, "scale+offset clamps at -12 V");
        check(clampRail(11.999f) == 11.999f, "the clamp does not engage below the rail");
    }

    {
        bool finite = true;
        for (float in : {-1e6f, 0.f, 1e6f})
            for (float att : {-1.f, 0.f, 1.f})
                for (float off : {-5.f, 5.f})
                    if (!std::isfinite(scaleOffset(in, att, off))) finite = false;
        check(finite, "output is finite even for absurd inputs");
    }

    std::printf(g_failures ? "\nFAILED (%d)\n" : "\nAll BlocCore tests passed\n",
                g_failures);
    return g_failures ? 1 : 0;
}
