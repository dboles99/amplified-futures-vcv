// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Shared by the Rack module and the commercial plug-in. The DSP was already
// public and MIT inside StringMassCore.cpp, so moving it into a header
// exposes nothing new; it just stops the two renderers being two
// implementations that happen to agree.
//
// sin and cos of a normalised phase, computed together.
//
// Measured motivation, not assumed. At 16 polyphonic channels of 16 voices
// the engine costs 110% of a core in UNIS and 303% in MICRO, so it does not
// run in real time at the polyphony the product claims. MICRO adds exactly
// one extra sin per voice per sample over UNIS and costs about 2.9 times as
// much, which points straight at the transcendentals.
//
// Two things make this worth doing rather than just calling std::sin twice:
//
//   1. sin and cos of the SAME angle share their range reduction, so
//      computing both together is close to the cost of one.
//   2. The caller already has a phase in [0, 1). Every libm sin starts by
//      reducing an arbitrary double-precision angle, which is work we can
//      skip entirely because our input is already bounded and normalised.
//
// Accuracy: octant reduction to [0, pi/4] followed by the standard Cephes
// single-precision minimax polynomials, which are good to roughly 1e-7
// absolute. That is about -140 dB, comfortably below anything audible, and
// well inside the 6e-06 relative error Rack already accepts from
// exp2_taylor5 in the same signal path.
//
// NOT BIT-IDENTICAL to std::sin and std::cos, and cannot be. Adopting this
// changes the output of any engine that uses it, which is why the switch is
// a deliberate decision recorded in a changelog rather than a silent
// optimisation. The equivalence test is what makes that decision safe to
// make: it fails the moment the two diverge, so the change cannot happen by
// accident.

#pragma once

#include <cmath>

namespace af {
namespace fasttrig {

// Cephes single-precision minimax coefficients, valid on [-pi/4, pi/4].
constexpr float SIN_C3 = -1.6666654611e-1f;
constexpr float SIN_C5 =  8.3321608736e-3f;
constexpr float SIN_C7 = -1.9515295891e-4f;

constexpr float COS_C2 = -4.9999999725e-1f;
constexpr float COS_C4 =  4.1666664571e-2f;
constexpr float COS_C6 = -1.3888781258e-3f;
constexpr float COS_C8 =  2.4433157513e-5f;

inline float sinPoly(float x) {
	const float x2 = x * x;
	return x + x * x2 * (SIN_C3 + x2 * (SIN_C5 + x2 * SIN_C7));
}

inline float cosPoly(float x) {
	const float x2 = x * x;
	return 1.f + x2 * (COS_C2 + x2 * (COS_C4 + x2 * (COS_C6 + x2 * COS_C8)));
}

// Given a phase in [0, 1), returns sin(2*pi*phase) and cos(2*pi*phase).
//
// Reduction is by octant rather than quadrant. A quadrant leaves the argument
// as large as pi/2, where a seventh-order sine is only good to about 1.6e-4,
// which is -76 dB and audible as distortion on a sustained tone. Folding to
// pi/4 costs one compare and buys three orders of magnitude.
inline void sincos2pi(float phase, float* sOut, float* cOut) {
	// Eighths of a turn.
	const float q = phase * 8.f;
	int oct = int(q);
	float f = q - float(oct);      // [0, 1) within the octant
	oct &= 7;

	// Odd octants run backwards from the next axis, so fold and swap.
	const bool swap = (oct & 1) != 0;
	if (swap)
		f = 1.f - f;

	const float x = f * (float(M_PI) / 4.f);   // [0, pi/4]
	float s = sinPoly(x);
	float c = cosPoly(x);

	if (swap) {
		const float t = s;
		s = c;
		c = t;
	}

	// Reconstruct the full turn from the octant. Written as a table rather
	// than nested branches because the pattern is easier to check by eye
	// against the unit circle than a chain of conditionals is.
	switch (oct) {
		case 0: *sOut =  s; *cOut =  c; break;
		case 1: *sOut =  s; *cOut =  c; break;
		case 2: *sOut =  c; *cOut = -s; break;
		case 3: *sOut =  c; *cOut = -s; break;
		case 4: *sOut = -s; *cOut = -c; break;
		case 5: *sOut = -s; *cOut = -c; break;
		case 6: *sOut = -c; *cOut =  s; break;
		default:*sOut = -c; *cOut =  s; break;
	}
}

}  // namespace fasttrig
}  // namespace af
