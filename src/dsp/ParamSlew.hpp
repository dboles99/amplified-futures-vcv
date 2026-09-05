#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// ParamSlew — a one-pole smoother for control values that multiply audio.
//
// A parameter that scales a signal must not step. A step in the gain is a
// click, and a parameter can move abruptly for reasons the module never sees:
// a DAW writing automation, a preset loading, a fast knob drag, a MIDI CC
// landing on a coarse 7-bit value.
//
// Rack has an engine-side smoother, but it is not the tool for this:
// ParamQuantity::setSmoothValue is deprecated ("identical to setValue since
// Rack 2.3.0"), the engine smooths exactly ONE parameter at a time and jumps
// any other to its target, and Knob::smooth already covers dragging. Smoothing
// inside the module works however the value arrives.
//
// No Rack dependency, so tests/test_param_slew.cpp can drive it directly.
// ============================================================
#include <cmath>

namespace ParamSlew {

/// One-pole smoother. `process` is allocation-free and branch-light.
class Smoother {
public:
    /// `ms` is the time to cover ~63% of a step. 2-10ms suits a gain: fast
    /// enough to feel immediate, slow enough that a jump stops being a click.
    void configure(float ms, float sampleRate) {
        const float tau = (ms > 0.f ? ms : 0.001f) * 0.001f;
        const float sr = (sampleRate > 1.f ? sampleRate : 44100.f);
        alpha = 1.f - std::exp(-1.f / (tau * sr));
        if (alpha > 1.f) alpha = 1.f;
        if (alpha < 0.f) alpha = 0.f;
    }

    /// First call jumps to the target rather than sliding up from zero.
    /// Without this every patch load fades in, which is a different defect
    /// from the one being fixed.
    float process(float target) {
        if (!primed) {
            primed = true;
            y = target;
            return y;
        }
        y += alpha * (target - y);
        return y;
    }

    /// Current value without advancing.
    float value() const { return y; }

    /// Drop the held value. The next process() jumps to its target, which is
    /// what a module reset or a fresh patch wants.
    void reset() {
        primed = false;
        y = 0.f;
    }

private:
    float y = 0.f;
    float alpha = 1.f;
    bool primed = false;
};

/// A fixed bank of smoothers configured together.
template <int N>
class Bank {
public:
    void configure(float ms, float sampleRate) {
        for (int i = 0; i < N; i++)
            s[i].configure(ms, sampleRate);
    }
    float process(int i, float target) { return s[i].process(target); }
    void reset() {
        for (int i = 0; i < N; i++)
            s[i].reset();
    }

private:
    Smoother s[N];
};

} // namespace ParamSlew
