// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// The DroneClone engine, lifted out of rack::Module.
//
// One instance is one string wall: eight voices detuned across a spread,
// each with its own slow drift LFO, a harmonic stack driven by TENSION and
// SHIMMER, a jawari bridge nonlinearity, and a sub an octave and a half down.
// A polyphonic wrapper holds one instance per channel.
//
// Shared by the Rack module and the commercial plug-in.
//
// PORTED, NOT REWRITTEN. Arithmetic and its order match DroneClone.cpp
// exactly, so the frozen reference in tests/golden_drone_data.hpp still
// holds. Improve it after the port is verified, not during.

#pragma once

#include "RackMath.hpp"

#include <algorithm>   // std::max, used by the jawari branch
#include <cmath>

namespace af {
namespace detail {

// Eight incommensurate rates in Hz, so the drift between voices never lines
// up on any short cycle. Multiplied by sampleTime in process(), which is
// what makes them mean Hz on every sample rate.
static const float DRONE_DRIFT_RATES[8] = {
	0.31f, 0.47f, 0.61f, 0.79f, 0.89f, 1.03f, 1.19f, 1.37f
};

// The jawari bridge offset, sin and cos of 0.02*pi.
//
// These were static locals inside process(), and that was a real defect
// rather than a style point. std::sin is not a constant expression, so the
// compiler had to emit a thread-safe initialisation guard, and that guard is
// a lock. It was acquired on the audio thread, on the first sample at which
// anyone turned JAWARI above zero. Found by -Wfunction-effects, which is
// exactly the class of thing it is for: a lock nobody would ever notice by
// listening, on a path taken once per session.
//
// At namespace scope they are initialised before main, off the audio thread,
// and the values are bit-identical, so the frozen reference still holds.
static const float DRONE_JAWARI_SIN = std::sin(0.02f * rackmath::PI);
static const float DRONE_JAWARI_COS = std::cos(0.02f * rackmath::PI);

}  // namespace detail

class DroneEngine {
public:
	static constexpr int VOICES = 8;

	struct Params {
		float spread   = 0.3f;   // 0 to 1, detune across the wall
		float mass     = 1.0f;   // 0 to 1, scaled to 0 to 8 voices by the caller
		float tension  = 0.f;    // harmonics 2 to 4
		float weight   = 0.f;    // sub, one octave and a fifth down
		float shimmer  = 0.f;    // harmonics 5 and 7
		float jawari   = 0.f;    // sitar bridge buzz
		float drift    = 0.f;    // depth of the per-voice drift
	};

	void setSampleRate(float sr) { sampleTime_ = 1.f / sr; }

	void reset() {
		for (int i = 0; i < VOICES; i++) {
			voice_[i] = Voice();
			driftPhase_[i] = 0.f;
		}
	}

	// basePitch is volts, 0 V being C4. massFloat is the voice count as a
	// continuous value so a voice fades in rather than switching on.
	float process(float basePitch, float massFloat, const Params& p) {
		using namespace rackmath;

		const float baseFreq = FREQ_C4 * exp2_taylor5(basePitch);
		float output = 0.f;

		for (int i = 0; i < VOICES; i++) {
			// A voice at the boundary is partly on, so MASS sweeps rather
			// than steps. The coefficient is per sample, which is a known
			// wart: the fade is faster at higher sample rates. Left alone
			// deliberately so this port stays checkable against the
			// reference; it is on the list, not forgotten.
			float targetLevel = (float(i) < massFloat) ? 1.f : 0.f;
			if (massFloat > float(i) && massFloat < float(i) + 1.f)
				targetLevel = massFloat - float(i);
			voice_[i].level += (targetLevel - voice_[i].level) * 4e-4f;

			if (voice_[i].level < 1e-4f) {
				voice_[i].wave = 0.f;
				continue;
			}

			const float normalizedPos = (float(i) / 7.f) - 0.5f;
			const float detuneCents   = normalizedPos * p.spread * 1200.f;
			float detuneFreq = baseFreq * exp2_taylor5(detuneCents / 1200.f);

			driftPhase_[i] += detail::DRONE_DRIFT_RATES[i] * sampleTime_;
			if (driftPhase_[i] >= 1.f) driftPhase_[i] -= 1.f;
			// At DRIFT 0 this multiplies by exactly 1, for the price of a sin.
			if (p.drift > 0.f)
				detuneFreq *= 1.f + p.drift * 0.008f *
				              std::sin(2.f * rackmath::PI * driftPhase_[i]);

			voice_[i].phase += detuneFreq * sampleTime_;
			if (voice_[i].phase >= 1.f) voice_[i].phase -= 1.f;

			// Every partial below is a harmonic of this one phase, so they all
			// follow from a single sin and cos by angle addition:
			//   s(n+1) = s(n)*c1 + c(n)*s1,  c(n+1) = c(n)*c1 - s(n)*s1
			// Exact, not an approximation, and it turns up to nine
			// transcendentals per voice into two.
			const float th = 2.f * rackmath::PI * voice_[i].phase;
			const float s1 = std::sin(th);
			const float c1 = std::cos(th);

			const float fundamental = s1;
			float toneSignal = fundamental;

			if (p.tension > 0.f || p.shimmer > 0.f) {
				const float s2 = 2.f * s1 * c1;
				const float c2 = c1 * c1 - s1 * s1;
				const float s3 = s2 * c1 + c2 * s1;
				const float c3 = c2 * c1 - s2 * s1;
				const float s4 = s3 * c1 + c3 * s1;

				if (p.tension > 0.f) {
					toneSignal += p.tension * 0.50f * s2;
					toneSignal += p.tension * 0.33f * s3;
					toneSignal += p.tension * 0.25f * s4;
					toneSignal /= 1.f + p.tension * 1.08f;
				}
				if (p.shimmer > 0.f) {
					const float c4 = c3 * c1 - s3 * s1;
					const float s5 = s4 * c1 + c4 * s1;
					const float c5 = c4 * c1 - s4 * s1;
					const float s6 = s5 * c1 + c5 * s1;
					const float c6 = c5 * c1 - s5 * s1;
					const float s7 = s6 * c1 + c6 * s1;
					toneSignal += p.shimmer * 0.12f * s5;
					toneSignal += p.shimmer * 0.08f * s7;
				}
			}

			if (p.jawari > 0.f) {
				// A sitar bridge only touches the string on one side of the
				// swing, so the buzz is a half-wave of a slightly shifted
				// copy. sin(th + 0.02*pi) by the same identity.
				const float buzz = std::max(0.f, s1 * detail::DRONE_JAWARI_COS +
				                                 c1 * detail::DRONE_JAWARI_SIN);
				toneSignal = toneSignal * (1.f - p.jawari * 0.35f) +
				             buzz * p.jawari * 0.35f;
			}

			float subSignal = 0.f;
			if (p.weight > 0.f) {
				voice_[i].subPhase += detuneFreq * 0.25f * sampleTime_;
				if (voice_[i].subPhase >= 1.f) voice_[i].subPhase -= 1.f;
				subSignal = p.weight * 0.4f *
				            std::sin(2.f * rackmath::PI * voice_[i].subPhase);
			}

			output += (toneSignal + subSignal) * voice_[i].level;
			voice_[i].wave = fundamental;
		}

		// Fixed divisor rather than 1/N, so MASS raises the level as well as
		// the voice count. Known, and deliberately unchanged by this port.
		return output / 8.f;
	}

	// For a panel that wants to show what each voice is doing. Not part of
	// the audio path, and a renderer with no lights can ignore both.
	float voiceLevel(int i) const { return voice_[i].level; }
	float voiceWave(int i)  const { return voice_[i].wave; }

private:
	struct Voice {
		float phase    = 0.f;
		float subPhase = 0.f;
		float level    = 0.f;
		float wave     = 0.f;   // last fundamental, for metering only
	};

	float sampleTime_ = 1.f / 44100.f;
	Voice voice_[VOICES];
	float driftPhase_[VOICES] = {};
};

}  // namespace af
