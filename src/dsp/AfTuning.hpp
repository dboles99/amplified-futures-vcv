#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

// ============================================================
// AfTuning — tuning and beating maths from the pitch research.
//
// Cents/ratio conversion, harmonic-series partials, the partial-spacing
// collapse, the beat-rate law and the vinyl wow law, as pure std-only
// functions. MIT and Rack-free so both the Branca modules and the
// proprietary JUCE line can consume it — the licence only permits that
// direction.
//
// The beat-rate law is the load-bearing one: beat_hz = f * cents / 1731.234,
// so any cents-denominated spread control changes character across the
// keyboard.
//
// Numbers are derived, not rounded.
// ============================================================

#include <cmath>

namespace af::tuning {

/// 1200 / ln(2). Converts a natural-log ratio to cents, and appears in every
/// small-deviation approximation below.
constexpr float kCentsPerLn2 = 1731.234049066756f;

[[nodiscard]] inline float centsToRatio(float cents) noexcept
{
    return std::pow(2.f, cents / 1200.f);
}

[[nodiscard]] inline float ratioToCents(float ratio) noexcept
{
    if (ratio <= 0.f) return 0.f;
    return 1200.f * std::log2(ratio);
}

/// Cents of partial `n` above the fundamental. This is the same relationship
/// the modules already use as `voct = root + log2(n)`, expressed in cents.
[[nodiscard]] inline float partialCents(int n) noexcept
{
    if (n < 1) return 0.f;
    return 1200.f * std::log2(static_cast<float>(n));
}

/// Cents between partial `n` and `n+1`.
///
/// This collapses fast — 53c at n=32, 27c at n=64, 14c at n=127. Above roughly
/// partial 30 the series is no longer a set of distinguishable pitches; it is a
/// beating field, and a control over it is a density control, not a pitch one.
[[nodiscard]] inline float partialSpacingCents(int n) noexcept
{
    if (n < 1) return 0.f;
    return 1200.f * std::log2(static_cast<float>(n + 1) / static_cast<float>(n));
}

/// Beat rate between two tones near `freqHz` separated by `detuneCents`.
///
/// Linear in frequency, so a fixed cents detune shimmers in the bass and
/// roughens in the treble: 10 cents is 0.6 Hz at 110 Hz and 5.1 Hz at 880 Hz.
/// Any SPREAD control denominated in cents inherits this behaviour, and any
/// control denominated in Hz instead breaks the tuning across the keyboard.
/// Neither is neutral. Pick one deliberately.
[[nodiscard]] inline float beatHz(float freqHz, float detuneCents) noexcept
{
    return freqHz * detuneCents / kCentsPerLn2;
}

/// Peak pitch deviation, in cents, from a vinyl pressing whose spindle hole is
/// `eccentricityMm` off centre, read at groove radius `radiusMm`.
///
/// Depth is radius-dependent, which is why a loop sampled near the run-out
/// wobbles more than one from the lead-in. Implement the expression, not a
/// constant.
[[nodiscard]] inline float vinylWowCents(float eccentricityMm,
                                         float radiusMm) noexcept
{
    if (radiusMm <= 0.f) return 0.f;
    return kCentsPerLn2 * eccentricityMm / radiusMm;
}

/// Wow rate: once per revolution. 0.556 Hz at 33 1/3, 0.75 Hz at 45.
[[nodiscard]] inline float vinylWowRateHz(float rpm) noexcept
{
    if (rpm <= 0.f) return 0.f;
    return rpm / 60.f;
}

} // namespace af::tuning
