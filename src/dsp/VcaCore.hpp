#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// VcaCore — gain and saturation maths for AF-05 Quad VCA.
//
// No Rack dependency, so it can be unit-tested by a standalone binary
// (tests/test_vca_core.cpp), as with ClockCore and EnvCore.
//
// Free functions rather than a stateful class: a VCA has no state worth
// keeping between samples. Channel normalling and summing belong to the
// Rack module, which is where the port-connected information lives.
//
// PRESSURE at 0 is bit-transparent — the same promise BROWNOUT and
// MISFIRE make on the clock and the envelope.
// ============================================================
#include <cmath>

namespace VcaCore {

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/// Map a 0..1 control value to a gain.
///
/// Linear is right for CV-controlled amplitude modulation; exponential
/// matches how loudness is perceived and is what a fader wants. Both pass
/// through (0,0) and (1,1) exactly, so switching curve cannot change the
/// level at either extreme — only the travel between them.
inline float gainCurve(float x, bool exponential) {
    const float c = clampf(x, 0.f, 1.f);
    if (!exponential) return c;
    if (c <= 0.f) return 0.f;
    if (c >= 1.f) return 1.f;
    // ~60 dB of usable range, anchored so f(1) == 1 exactly.
    const float minDb = -60.f;
    const float db = minDb * (1.f - c);
    return std::pow(10.f, db / 20.f);
}

/// Soft saturation applied to the summed output.
///
/// `pressure` 0..1. At exactly 0 the input is returned unchanged — not
/// "almost unchanged": a mixer that colours the signal when its character
/// knob is down cannot be used as a clean mixer.
///
/// tanh is odd-symmetric, so this adds no DC offset. An asymmetric curve
/// would bias the sum and show up as offset in a render.
inline float saturate(float x, float pressure) {
    if (pressure <= 0.f) return x;
    const float p = clampf(pressure, 0.f, 1.f);
    const float drive = 1.f + p * 4.f;
    // Divide by tanh(drive) so full-scale in stays full-scale out: the knob
    // adds harmonics rather than simply turning the signal down.
    //
    // That normalisation raises the ceiling to 1/tanh(drive), which at low
    // pressure is 1.13 - so a hot input would overshoot and PRESSURE would
    // fail as a safety. Clamp it. The clamp only engages beyond full scale,
    // where the signal is already past where a saturator should let it go.
    return clampf(std::tanh(x * drive) / std::tanh(drive), -1.f, 1.f);
}

} // namespace VcaCore
