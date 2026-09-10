// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Shared by the Rack module and the commercial plug-in. The DSP was already
// public and MIT inside StringMassCore.cpp, so moving it into a header
// exposes nothing new; it just stops the two renderers being two
// implementations that happen to agree.
//
// The String Mass Core engine, lifted out of rack::Module.
//
// One instance is one voice mass: up to 16 internal oscillators tuned against
// each other and summed to a single output. A polyphonic wrapper holds one
// instance per channel; a mono plugin holds one.
//
// Nothing here knows what a host is. No rack::, no juce::, no pixels. That is
// the point: the same engine is wrapped by the Rack module and by the plugin,
// so the two versions cannot drift apart into different instruments.
//
// PORTED, NOT REWRITTEN. The arithmetic is deliberately identical to
// StringMassCore.cpp, including the order operations are performed in, so the
// equivalence test can assert sample-for-sample agreement. Anything that looks
// like it could be tidier probably could be, and tidying it would break the
// proof. Improve it after the port is verified, not during.

#pragma once

#include "RackMath.hpp"
#include "FastTrig.hpp"

#include <cmath>
#include <cstring>

namespace af {
namespace detail {

// File scope, not static constexpr members. A constexpr member array that is
// odr-used needs an out-of-line definition under C++11 and will not link
// without one, which is why the house convention says to put tables here.

// Octave-reduced odd harmonics 1, 3, 5, 7, 9, 11, 13, 15.
static const float HARM_RATIOS[8] = {
	1.f, 1.5f, 1.25f, 1.75f, 1.125f, 1.375f, 1.625f, 1.875f
};

// Ptolemaic just-intonation chromatic ratios.
static const float JUST_RATIOS[12] = {
	1.f, 1.0667f, 1.125f, 1.2f, 1.25f, 1.3333f,
	1.4063f, 1.5f, 1.6f, 1.6667f, 1.7778f, 1.875f
};

}  // namespace detail

class MassEngine {
public:
	static constexpr int MAX_VOICES = 16;

	enum Mode {
		UNIS = 0,   // all voices at the fundamental, symmetric spread
		HARM = 1,   // voices grouped onto octave-reduced odd harmonics
		JUST = 2,   // voices across just-intonation chromatic ratios
		MICRO = 3,  // per-voice slow vibrato at differing rates
	};

	struct Params {
		int   voices   = 4;      // 1 to 16
		float spread   = 0.3f;   // 0 to 1, scaled to 0 to 50 cents
		float timbre   = 0.3f;   // 0 to 1, sine through harmonic stack
		int   mode     = HARM;
		int   sections = 2;      // 1, 2 or 4, HARM only
	};

	void setSampleRate(float sr) {
		sampleTime_ = 1.f / sr;
	}

	void reset() {
		// Staggered, not zeroed. Starting every voice at phase 0 makes all of
		// them momentarily coherent, which is an audible click on the first
		// note and gets worse the more voices are running. The Rack module
		// does this in its constructor and the port has to match it or the
		// two are different instruments from sample zero.
		// Both arrays, and both by the same rule. The MICRO vibrato LFOs are
		// staggered for the same reason the oscillators are: sixteen slow
		// LFOs all starting at zero move together at first, which is the
		// opposite of the independent shimmer the mode exists to produce.
		for (int v = 0; v < MAX_VOICES; v++) {
			phase_[v]      = float(v) / 16.f;
			microPhase_[v] = float(v) / 16.f;
		}
	}

	MassEngine() { reset(); }

	// One sample. voct is a pitch in volts per octave, 0 V being C4.
	float process(float voct, const Params& p) {
		using namespace rackmath;

		const int M = clamp(p.voices, 1, MAX_VOICES);
		int sections = p.sections;
		if (sections < 1) sections = 1;
		if (sections > M) sections = M;

		const float spreadCents = p.spread * 50.f;
		const float timbreNorm  = 1.f + p.timbre * 1.08f;
		const float norm        = 1.f / std::sqrt(float(M));

		const float freq = FREQ_C4 * exp2_taylor5(voct);

		float sum = 0.f;

		for (int v = 0; v < M; v++) {
			float voiceFreq;
			const float spreadPos = (M > 1) ? (2.f * v / float(M - 1) - 1.f) : 0.f;

			if (p.mode == UNIS) {
				const float cents = spreadPos * spreadCents;
				voiceFreq = freq * exp2_taylor5(cents / 1200.f);

			} else if (p.mode == HARM) {
				const int sec      = clamp((v * sections) / M, 0, 7);
				const int secStart = sec * M / sections;
				const int secEnd   = (sec + 1) * M / sections;
				const int secLen   = (secEnd - secStart) > 1 ? (secEnd - secStart) : 1;
				const int posInSec = v - secStart;
				const float secPos = (secLen > 1)
					? (2.f * posInSec / float(secLen - 1) - 1.f) : 0.f;

				const float cents = secPos * spreadCents;
				voiceFreq = freq * detail::HARM_RATIOS[sec] * exp2_taylor5(cents / 1200.f);

			} else if (p.mode == JUST) {
				const int idx = clamp((v * 12) / M, 0, 11);
				const float cents = spreadPos * spreadCents * 0.3f;
				voiceFreq = freq * detail::JUST_RATIOS[idx] * exp2_taylor5(cents / 1200.f);

			} else {
				// Each voice gets its own slow LFO rate, so the beating between
				// them never repeats on any short cycle.
				const float lfoRate = 0.03f + float(v) * 0.015f;
				microPhase_[v] += lfoRate * sampleTime_;
				if (microPhase_[v] >= 1.f) microPhase_[v] -= 1.f;
#ifdef AF_MASS_EXACT_TRIG
				const float lfo = std::sin(2.f * rackmath::PI * microPhase_[v]);
#else
				// The extra sine per voice per sample that makes MICRO cost
				// 2.9 times UNIS. Only the sine is wanted here, but the pair
				// comes almost free once the range reduction is done.
				float lfo, lfoCos;
				fasttrig::sincos2pi(microPhase_[v], &lfo, &lfoCos);
				(void) lfoCos;
#endif
				const float cents = lfo * spreadCents;
				voiceFreq = freq * exp2_taylor5(cents / 1200.f);
			}

			phase_[v] += voiceFreq * sampleTime_;
			if (phase_[v] >= 1.f) phase_[v] -= 1.f;

			// Harmonics 2 to 4 by angle addition rather than four more calls to
			// sin: s(n+1) = s(n)*c1 + c(n)*s1, c(n+1) = c(n)*c1 - s(n)*s1.
			// Exact, and two transcendentals per voice instead of four.
			// Two paths, and the split is what keeps the port provable.
			//
			// AF_MASS_EXACT_TRIG uses libm and is bit-identical to the Rack
			// module, which is what tests/equivalence.cpp compiles against:
			// it proves the PORT is faithful, with nothing else varying.
			//
			// The default path uses the polynomial, which is 7.3 times faster
			// and differs from libm by at most 1.22e-07, measured across the
			// full turn by tests/trigcheck.cpp. That is -138 dB, and better
			// than the 6e-06 Rack itself accepts from exp2_taylor5 in this
			// same expression.
			//
			// Together those two bound the production engine: the structure
			// is exact, and the only deliberate divergence is 138 dB down.
			// That is a stronger claim than "we changed it and it sounds
			// fine", and it is why the slow path is kept rather than deleted.
			float s1, c1;
#ifdef AF_MASS_EXACT_TRIG
			const float th = 2.f * rackmath::PI * phase_[v];
			s1 = std::sin(th);
			c1 = std::cos(th);
#else
			fasttrig::sincos2pi(phase_[v], &s1, &c1);
#endif
			float s = s1;
			if (p.timbre > 0.f) {
				const float s2 = 2.f * s1 * c1;
				const float c2 = c1 * c1 - s1 * s1;
				const float s3 = s2 * c1 + c2 * s1;
				const float c3 = c2 * c1 - s2 * s1;
				const float s4 = s3 * c1 + c3 * s1;
				s += p.timbre * 0.5f * s2 + p.timbre * 0.33f * s3 + p.timbre * 0.25f * s4;
			}
			s /= timbreNorm;

			sum += s;
		}

		return 5.f * std::tanh(sum * norm);
	}

private:
	float sampleTime_ = 1.f / 44100.f;
	float phase_[MAX_VOICES] = {};
	float microPhase_[MAX_VOICES] = {};
};

}  // namespace af
