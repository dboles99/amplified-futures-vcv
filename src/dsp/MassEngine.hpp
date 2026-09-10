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

#include <algorithm>   // std::max, used by the jawari branch
#include <cmath>
#include <cstring>

namespace af {
namespace detail {

// File scope, not static constexpr members. A constexpr member array that is
// odr-used needs an out-of-line definition under C++11 and will not link
// without one, which is why the house convention says to put tables here.

// -6 dB as an amplitude ratio: the level of the topmost field member
// relative to the bottom one when taper is at full.
static const float TAPER_FLOOR = 0.5011872336f;

// Octave-reduced odd harmonics 1, 3, 5, 7, 9, 11, 13, 15.
static const float HARM_RATIOS[8] = {
	1.f, 1.5f, 1.25f, 1.75f, 1.125f, 1.375f, 1.625f, 1.875f
};

// The jawari bridge offset, sin and cos of 0.02*pi, at namespace scope
// because a static local initialised by std::sin carries a thread-safe
// initialisation guard and that guard is a lock on the audio thread. Same
// value and same reason as DroneEngine's.
static const float JAWARI_SIN = std::sin(0.02f * rackmath::PI);
static const float JAWARI_COS = std::cos(0.02f * rackmath::PI);

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

		// The three below are additive: every one of them is inert at its
		// default, so a caller that does not know about them - the Rack
		// module - gets bit-identical output. Verified rather than assumed.

		// Spread stated directly in cents, overriding the 0-to-50 range that
		// `spread` maps onto. Negative means "use spread", which is what the
		// Rack module does and must keep doing.
		//
		// The Rack knob covers 50 cents because that is the useful range for
		// one module in a patch. The instrument needs the whole way to an
		// octave: its SPREAD macro walks four named perceptual regimes and
		// three of them live above 60 cents, so a 50-cent ceiling would put
		// the flagship control entirely inside the first one.
		float spreadCents = -1.f;

		// Level rolloff from the lowest member of the field to the highest.
		// At 1 the bottom sits about 6 dB above the top. At 0 every member is
		// at the same level, which is a multiply by exactly one.
		float taper = 0.f;

		// Which harmonic the field is built on. Above 1 the fundamental is
		// absent and the ear infers it from the partials that remain, which
		// is what a third bridge does to a string. 1 is a multiply by
		// exactly one.
		float firstPartial = 1.f;

		// Partials 5 and 7, which sit outside the 2-3-4 stack that `timbre`
		// weights and read as air rather than as body. Same coefficients as
		// DroneClone, which is where this came from.
		float shimmer = 0.f;

		// Sitar bridge buzz. A string in unresolved contact with a curved
		// bridge only touches on one side of the swing, so the buzz is a
		// half-wave of a slightly phase-shifted copy.
		float jawari = 0.f;

		// Slow detuning of the members away from where they were put.
		//
		// Coherence is the interesting one and it is why this is here rather
		// than as a modulation route. At 1 the whole field drifts as a body,
		// which reads as an ensemble going out of tune with the room; at 0
		// each member drifts alone, which reads as an ensemble that is alive.
		// Same depth and the same rate family, opposite musical meaning.
		//
		// Every drift path is skipped entirely at depth 0, so the default
		// costs nothing and changes nothing.
		float driftDepth     = 0.f;    // 0 to 1, about 40 cents at full
		float driftRate      = 0.02f;  // Hz
		float driftCoherence = 1.f;    // 1 together, 0 independent
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
			// Staggered for the third time and for the third instance of
			// the same reason: sixteen drift LFOs starting together produce
			// one coherent swoop, which is what coherence 0 exists to avoid.
			driftPhase_[v] = float(v) / 16.f;
		}
		sharedDriftPhase_ = 0.f;
	}

	MassEngine() { reset(); }

	// One sample. voct is a pitch in volts per octave, 0 V being C4.
	//
	// quads, when given, receives the field split into four equal groups of
	// members, low to high, and the four sum to the return value. It is what
	// the wall's four channels are: the design's ruling is that Mass Driver's
	// sixteen channels "are our sixteen field members", so the wall's density
	// sweeps parts of the field rather than parts of the polyphony. Feeding
	// it whole voices instead would make density mean something different
	// depending on how many notes were held, which is not a control.
	//
	// nullptr costs one predictable branch per member and changes nothing:
	// the return value is accumulated in the same order either way.
	float process(float voct, const Params& p, float* quads = nullptr) {
		using namespace rackmath;

		const int M = clamp(p.voices, 1, MAX_VOICES);
		int sections = p.sections;
		if (sections < 1) sections = 1;
		if (sections > M) sections = M;

		// Negative means the caller did not state cents, so derive them the
		// way the Rack module always has.
		const float spreadCents = (p.spreadCents >= 0.f) ? p.spreadCents
		                                                 : p.spread * 50.f;
		const float timbreNorm  = 1.f + p.timbre * 1.08f;
		const float norm        = 1.f / std::sqrt(float(M));

		// At firstPartial 1 this is a multiply by exactly one, which is why
		// it can sit on the hot path without changing the reference output.
		const float freq = FREQ_C4 * exp2_taylor5(voct) * p.firstPartial;

		// Taper is the depth of a linear ramp from the bottom member to the
		// top. TAPER_FLOOR is -6 dB as an amplitude, so at full taper the top
		// member is 6 dB down on the bottom one. At taper 0 every member gets
		// exactly 1.f.
		const float taperDrop = p.taper * (1.f - detail::TAPER_FLOOR);

		// One shared LFO for the whole field, advanced once per sample
		// rather than once per member, which is what makes coherence 1 mean
		// "as a body" rather than "sixteen LFOs that happen to agree".
		if (p.driftDepth > 0.f) {
			sharedDriftPhase_ += p.driftRate * sampleTime_;
			if (sharedDriftPhase_ >= 1.f) sharedDriftPhase_ -= 1.f;
			// Kept so the modulation matrix can use the field's own drift as
			// a source rather than a second LFO that merely runs at the same
			// rate. Written, never read, by the audio path itself.
			float c;
			fasttrig::sincos2pi(sharedDriftPhase_, &sharedDrift_, &c);
			(void) c;
		} else {
			sharedDrift_ = 0.f;
		}

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

			// Applied on top of whatever the distribution chose, so it works
			// the same way in all four of them. Skipped whole at depth 0,
			// which is what keeps the reference output exact.
			if (p.driftDepth > 0.f) {
				// Incommensurate per-member rates, so the members never line
				// up on a short cycle however long the drift runs.
				const float ownRate = p.driftRate * (0.71f + 0.043f * float(v));
				driftPhase_[v] += ownRate * sampleTime_;
				if (driftPhase_[v] >= 1.f) driftPhase_[v] -= 1.f;

				float own, ownCos, shared, sharedCos;
				fasttrig::sincos2pi(driftPhase_[v],   &own,    &ownCos);
				fasttrig::sincos2pi(sharedDriftPhase_, &shared, &sharedCos);
				(void) ownCos; (void) sharedCos;

				const float lfo = p.driftCoherence * shared +
				                  (1.f - p.driftCoherence) * own;
				voiceFreq *= exp2_taylor5(p.driftDepth * 40.f * lfo / 1200.f);
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

				if (p.shimmer > 0.f) {
					const float c4 = c3 * c1 - s3 * s1;
					const float s5 = s4 * c1 + c4 * s1;
					const float c5 = c4 * c1 - s4 * s1;
					const float s6 = s5 * c1 + c5 * s1;
					const float c6 = c5 * c1 - s5 * s1;
					const float s7 = s6 * c1 + c6 * s1;
					s += p.shimmer * 0.12f * s5 + p.shimmer * 0.08f * s7;
				}
			}
			s /= timbreNorm;

			if (p.jawari > 0.f) {
				const float buzz = std::max(0.f, s1 * detail::JAWARI_COS +
				                                 c1 * detail::JAWARI_SIN);
				s = s * (1.f - p.jawari * 0.35f) + buzz * p.jawari * 0.35f;
			}

			// Exactly 1.f when taper is 0, so the reference output holds.
			const float memberLevel = (M > 1)
				? 1.f - taperDrop * (float(v) / float(M - 1))
				: 1.f;

			const float member = s * memberLevel;
			sum += member;
			if (quads) {
				int q = (M > 1) ? (v * 4) / M : 0;
				if (q > 3) q = 3;
				quads[q] += member * norm;
			}
		}

		return 5.f * std::tanh(sum * norm);
	}

	// The field's shared drift LFO, bipolar, for the modulation matrix
	// and for a display. Zero when drift depth is zero, because then
	// there is no drift to be the source of.
	float sharedDrift() const { return sharedDrift_; }

private:
	float sampleTime_ = 1.f / 44100.f;
	float phase_[MAX_VOICES] = {};
	float microPhase_[MAX_VOICES] = {};
	float driftPhase_[MAX_VOICES] = {};
	float sharedDriftPhase_ = 0.f;
	float sharedDrift_ = 0.f;
};

}  // namespace af
