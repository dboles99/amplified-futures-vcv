#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// BlocCore — CV arithmetic for AF-06 Signal Bloc.
//
// Small, but the smallness is the point: this is the one module in the series
// that must not colour anything. A utility that is almost transparent is worse
// than no utility, because you cannot tell which stage moved your voltage.
// So the promises here are exact, and tests/test_bloc_core.cpp checks them
// with == rather than a tolerance.
//
// No Rack dependency, in the same shape as ClockCore, EnvCore, VcaCore
// and RatchetCore.
// ============================================================
#include <cmath>

namespace BlocCore {

/// Rack's convention: modules must not emit beyond +/-12 V, whatever the maths
/// says. Everything that leaves this module goes through here.
inline float clampRail(float v) {
    return v < -12.f ? -12.f : (v > 12.f ? 12.f : v);
}

/// One attenuverter-plus-offset channel: out = in * att + off.
///
/// At att == 1 and off == 0 the input is returned bit-identically, not
/// "within an epsilon" - multiplying by a float 1.f and adding a float 0.f
/// is exact, and the early return makes that a guarantee rather than a
/// property of the optimiser.
inline float scaleOffset(float in, float att, float off) {
    if (att == 1.f && off == 0.f) return clampRail(in);
    return clampRail(in * att + off);
}

/// Precision three-input adder. No saturation: a summing utility that softens
/// its peaks cannot be used to build a control voltage you can predict.
/// The rail clamp is the only nonlinearity, and it sits where Rack requires.
inline float sum3(float a, float b, float c) {
    return clampRail(a + b + c);
}

} // namespace BlocCore
