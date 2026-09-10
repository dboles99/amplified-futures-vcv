// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// The Wall Conductor engine, lifted out of rack::Module.
//
// Four channels swept in and out by one density control, panned to a fixed
// spread, driven into saturation by pressure, fed back through a DC-blocked
// bus, and gated by a collapse envelope with a shaped recovery.
//
// Shared by the Rack module and the commercial plug-in, so the two renderers
// are one instrument rather than two implementations that agree today.
//
// PORTED, NOT REWRITTEN. The arithmetic and its order match WallConductor.cpp
// exactly, so the frozen reference in tests/golden_wall_data.hpp still holds.
// Tidy it after the port is verified, not during.

#pragma once

#include "RackMath.hpp"

#include <cmath>

namespace af {
namespace detail {

// Fixed channel positions, left to right. At namespace scope rather than as a
// static local inside process(), because a static local is a lock: the
// compiler guards its initialisation so that two threads cannot race to run
// it. These initialisers are constant so no guard is actually emitted, but
// -Wfunction-effects reasons about the declaration rather than the codegen and
// will not certify a function containing one as non-blocking. Hoisting it is
// free and keeps the certification, which is worth more than the locality.
static const float WALL_PANS[4] = { -1.f, -0.33f, 0.33f, 1.f };

}  // namespace detail

class WallEngine {
public:
	struct Params {
		float density  = 1.0f;   // 0 to 1, sweeps four channels in
		float pressure = 0.3f;   // 0 to 1, drive into the saturator
		float width    = 0.7f;   // 0 to 1, stereo spread
		float feedback = 0.0f;   // 0 to 0.92, clamped by the caller
		float recovery = 0.3f;   // 0 to 1, collapse recovery time
		bool  collapse = false;
	};

	void setSampleRate(float sr) {
		sampleTime_ = 1.f / sr;
		// Constant for a given rate, so it is computed once rather than per
		// sample. Same value either way; exp is deterministic.
		hpAlpha_ = 1.f - std::exp(-2.f * rackmath::PI * 5.f * sampleTime_);
	}

	void reset() {
		collapseEnv_ = 1.f;
		feedbackL_ = feedbackR_ = 0.f;
		fbHpL_ = fbHpR_ = 0.f;
	}

	// ch holds the four channel inputs, already summed across any polyphony
	// by the caller. Summing is a host concern; this is not.
	void process(const float ch[4], const Params& p, float* outLp, float* outRp) {
		// Collapse falls fast and recovers slowly, which is the gesture: the
		// wall drops out at once and comes back at a speed you choose.
		const float collapseTarget = p.collapse ? 0.f : 1.f;
		const float attackTime   = 0.002f;
		const float recoveryTime = 0.05f + p.recovery * 9.95f;
		const float tau = (collapseTarget < collapseEnv_) ? attackTime : recoveryTime;
		collapseEnv_ += (collapseTarget - collapseEnv_) *
		                (1.f - std::exp(-sampleTime_ / tau));

		// Width scales the spread rather than moving individual channels, so
		// the shape of the wall is constant and only its breadth changes.
		float mixL = feedbackL_ * p.feedback;
		float mixR = feedbackR_ * p.feedback;

		for (int i = 0; i < 4; i++) {
			// Channels come in one at a time as density rises, rather than
			// all four fading together. Density is a count, not a volume.
			const float gain = clampf(p.density * 4.f - float(i), 0.f, 1.f);
			const float sig  = ch[i] * gain;
			const float panR = (detail::WALL_PANS[i] * p.width + 1.f) * 0.5f;
			mixL += sig * std::cos(panR * rackmath::PI * 0.5f);
			mixR += sig * std::sin(panR * rackmath::PI * 0.5f);
		}

		const float drive = 1.f + p.pressure * 3.f;
		const float outL = 5.f * std::tanh(mixL * drive / 5.f) * collapseEnv_;
		const float outR = 5.f * std::tanh(mixR * drive / 5.f) * collapseEnv_;

		// The feedback bus is DC-blocked before it re-enters the mix.
		//
		// Without this the autonomous loop is y[n] = 5*tanh(f*drive*y[n-1]/5),
		// whose origin is unstable whenever feedback*(1 + 3*pressure) > 1. At
		// feedback maximum that is pressure above 0.029, three percent into
		// the knob. Past it the loop converged on a non-zero DC fixed point
		// and sat there: 4.47 V at pressure 0.25, 4.99 V at maximum, reached
		// within ten samples from silence, and completely silent.
		//
		// Blocking DC does not remove the instability and is not meant to.
		// Past the same boundary the loop oscillates instead of latching,
		// which is what controlled feedback is for.
		fbHpL_ += hpAlpha_ * (outL - fbHpL_);
		fbHpR_ += hpAlpha_ * (outR - fbHpR_);
		feedbackL_ = outL - fbHpL_;
		feedbackR_ = outR - fbHpR_;

		*outLp = outL;
		*outRp = outR;
	}

	// Exposed so a UI can show how close the loop is to self-oscillation.
	// Below 1 the loop decays, at and above it diverges: a change of sign
	// rather than of degree.
	static float loopGain(const Params& p) {
		return p.feedback * (1.f + 3.f * p.pressure);
	}

private:
	static float clampf(float x, float lo, float hi) {
		return x < lo ? lo : (x > hi ? hi : x);
	}

	float sampleTime_  = 1.f / 44100.f;
	float hpAlpha_     = 0.f;
	float collapseEnv_ = 1.f;
	float feedbackL_   = 0.f;
	float feedbackR_   = 0.f;
	float fbHpL_       = 0.f;
	float fbHpR_       = 0.f;
};

}  // namespace af
