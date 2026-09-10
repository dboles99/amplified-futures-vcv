// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Shared by the Rack module and the commercial plug-in. The DSP was already
// public and MIT inside StringMassCore.cpp, so moving it into a header
// exposes nothing new; it just stops the two renderers being two
// implementations that happen to agree.
//
// Bit-exact reimplementations of the handful of Rack DSP helpers the Branca
// engines call.
//
// These exist so the extracted engines produce output identical to the Rack
// modules, sample for sample, rather than merely similar. Substituting
// std::exp2 here would be correct to about six decimal places and would make
// the port impossible to verify: every difference would then be either a
// porting bug or the approximation, with no way to tell which.
//
// Ported from the Rack SDK 2.6.6 headers, which are the reference. If the SDK
// changes these, the equivalence test fails and this file needs revisiting.

#pragma once

#include <cstdint>

namespace af {
namespace rackmath {

// rack::dsp::FREQ_C4
constexpr float FREQ_C4 = 261.6256f;

template <typename T>
inline T clamp(T x, T lo, T hi) {
	return x < lo ? lo : (x > hi ? hi : x);
}

// rack::dsp::exp2Floor
//
// Returns 2^floor(x) by writing the exponent straight into the mantissa field
// of a float, and hands back the fractional part. Adding 127 first is what
// makes the int conversion truncate toward negative infinity for the input
// range that matters, and it is also the float exponent bias, which is why the
// shift by 23 lands the value in the exponent field.
inline float exp2Floor(float x, float* xf) {
	x += 127.f;
	const int32_t xi = static_cast<int32_t>(x);
	if (xf)
		*xf = x - static_cast<float>(xi);
	union {
		float yi;
		int32_t yii;
	};
	yii = xi << 23;
	return yi;
}

// rack::dsp::exp2_taylor5
//
// 2^x to at most 6e-06 relative error. Coefficients are Rack's, credited there
// to Andy Simper, chosen to minimise relative error while staying continuous
// and exact at integer x.
//
// The coefficients MUST be float here. Rack declares them as `const T a[]`
// with T deduced as float, so the double literals in its source are truncated
// to float before use. Keeping them double and letting the multiply promote
// would give a different, slightly better answer, and a different answer is
// exactly what this file exists to avoid.
inline float exp2_taylor5(float x) {
	float xf = 0.f;
	const float yi = exp2Floor(x, &xf);

	const float a[6] = {
		1.0f,
		0.69315169353961f,
		0.2401595990753f,
		0.055817908652f,
		0.008991698010f,
		0.001879100722f,
	};

	// Horner, evaluated in the same order as rack::dsp::polyHorner. Order
	// matters: floating point addition is not associative, so evaluating this
	// as Estrin's method or as a plain sum would give a different last bit.
	float y = a[5];
	for (int n = 1; n < 6; n++)
		y = a[5 - n] + y * xf;

	return yi * y;
}

}  // namespace rackmath
}  // namespace af
