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

		// Everything below is additive and inert at its default, so the Rack
		// module - which sets none of it - is unchanged and the frozen
		// reference still holds. Verified bit for bit, not assumed.
		//
		// Each one is also SKIPPED rather than merely neutral at its default.
		// A tilt filter set to unity gain still runs its state update, and
		// "multiply by a number that rounds to one" is not the same as "do
		// not run", which is the difference between the reference holding
		// and the reference almost holding.

		// Which saturation curve, from Collapse Saturator, by its own names.
		// 0 is the symmetric tanh the wall has always used.
		int character = 0;      // 0 odd, 1 even (tape), 2 full (hard clip)

		// Broadband spectral tilt about 90 Hz. Negative leans low, which is
		// the direction the WEIGHT macro drives it.
		float tilt = 0.f;       // -1 to 1

		// Damping in the return path, from Feedback Governor's TONE. 1 is
		// full bandwidth, which is what the wall has always done.
		float loopTone = 1.f;   // 0 dark (100 Hz) to 1 open (20 kHz)

		// How far a collapse ducks. 1 is silence, which is the only
		// behaviour there has been until now.
		float collapseDepth = 1.f;

		// Where the governor starts holding the loop back, as a fraction of
		// full scale. Negative disables it entirely, which is what the Rack
		// module wants: its governor is a separate module and a patch that
		// does not include one should not get one for free.
		float governor = -1.f;
	};

	void setSampleRate(float sr) {
		sampleTime_ = 1.f / sr;
		// Constant for a given rate, so it is computed once rather than per
		// sample. Same value either way; exp is deterministic.
		hpAlpha_ = 1.f - std::exp(-2.f * rackmath::PI * 5.f * sampleTime_);

		// The tilt crossover, at 90 Hz.
		tiltAlpha_ = 1.f - std::exp(-2.f * rackmath::PI * 90.f * sampleTime_);

		// 5 ms to see it, 200 ms to forgive it. A loop that is building
		// needs catching quickly; one that has stopped should be let go
		// slowly, or the governor pumps.
		envAttack_  = 1.f - std::exp(-sampleTime_ / 0.005f);
		envRelease_ = 1.f - std::exp(-sampleTime_ / 0.200f);
		govAttack_  = 1.f - std::exp(-sampleTime_ / 0.002f);
		govRelease_ = 1.f - std::exp(-sampleTime_ / 0.300f);
	}

	void reset() {
		collapseEnv_ = 1.f;
		feedbackL_ = feedbackR_ = 0.f;
		fbHpL_ = fbHpR_ = 0.f;
		fbLpL_ = fbLpR_ = 0.f;
		tiltL_ = tiltR_ = 0.f;
		loopEnv_ = 0.f;
		governorGain_ = 1.f;
	}

	// ch holds the four channel inputs, already summed across any polyphony
	// by the caller. Summing is a host concern; this is not.
	void process(const float ch[4], const Params& p, float* outLp, float* outRp) {
		// Collapse falls fast and recovers slowly, which is the gesture: the
		// wall drops out at once and comes back at a speed you choose.
		// Depth 1 gives exactly 0, which is the only behaviour there was.
		const float collapseTarget = p.collapse ? (1.f - p.collapseDepth) : 1.f;
		const float attackTime   = 0.002f;
		const float recoveryTime = 0.05f + p.recovery * 9.95f;
		const float tau = (collapseTarget < collapseEnv_) ? attackTime : recoveryTime;
		collapseEnv_ += (collapseTarget - collapseEnv_) *
		                (1.f - std::exp(-sampleTime_ / tau));

		// Width scales the spread rather than moving individual channels, so
		// the shape of the wall is constant and only its breadth changes.
		// The governor holds the loop at its threshold rather than letting it
		// run away. Disabled by default and skipped whole when it is, so the
		// Rack module - whose governor is a separate patchable module - does
		// not silently acquire one.
		float loopGainNow = p.feedback;
		if (p.governor >= 0.f) {
			// Fast to catch, slow to let go, so a transient does not duck the
			// whole wall and a sustained build is actually held.
			const float target = loopEnv_ > 0.f ? loopEnv_ : 0.f;

			// Exponential, 0.1 V to 5 V, not linear.
			//
			// The first version was linear from 0.05 to 5 V and its default
			// sat at 2.5 V, which the wall reaches only when it is already
			// out of control. The dead-control test caught it: a governor
			// that never engages at any setting a user would choose is not
			// a control, and it was found by measurement rather than by
			// listening for something that never happened.
			//
			// Exponential puts the default at about 0.7 V, which is a real
			// leash at ordinary levels, and still opens all the way to 5 V.
			const float ceiling = 0.1f * std::pow(50.f, p.governor);
			const float over = (ceiling > 0.f) ? target / ceiling : 0.f;
			const float wanted = (over > 1.f) ? 1.f / over : 1.f;
			const float coeff = (wanted < governorGain_) ? govAttack_ : govRelease_;
			governorGain_ += (wanted - governorGain_) * coeff;
			loopGainNow *= governorGain_;
		}

		float mixL = feedbackL_ * loopGainNow;
		float mixR = feedbackR_ * loopGainNow;

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
		float outL = 5.f * saturate(mixL * drive / 5.f, p.character) * collapseEnv_;
		float outR = 5.f * saturate(mixR * drive / 5.f, p.character) * collapseEnv_;

		// Broadband tilt about 90 Hz. A one-pole split, so low plus high is
		// the input again and tilt 0 would be a no-op anyway - but it is
		// skipped rather than computed, because "adds back to the same
		// number" and "is the same number" are different in floating point.
		if (p.tilt != 0.f) {
			tiltL_ += tiltAlpha_ * (outL - tiltL_);
			tiltR_ += tiltAlpha_ * (outR - tiltR_);
			// About 5 dB either way at the extremes.
			const float lowGain  = 1.f - p.tilt * 0.44f;
			const float highGain = 1.f + p.tilt * 0.44f;
			outL = tiltL_ * lowGain + (outL - tiltL_) * highGain;
			outR = tiltR_ * lowGain + (outR - tiltR_) * highGain;
		}

		// The loop's energy, for the governor and for the stability display.
		// Peak-ish rather than RMS, because what matters is how close the
		// loop is to running away and RMS understates a spike.
		{
			const float mag = (std::fabs(outL) > std::fabs(outR))
				? std::fabs(outL) : std::fabs(outR);
			const float coeff = (mag > loopEnv_) ? envAttack_ : envRelease_;
			loopEnv_ += (mag - loopEnv_) * coeff;
		}

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
		float retL = outL, retR = outR;

		// Damping in the return path, from Feedback Governor's TONE, with
		// the same 100 Hz to 20 kHz exponential sweep. Skipped at 1, which
		// is what the wall has always done: no damping at all.
		//
		// This is what stops a loop building into noise, as distinct from
		// what stops it running away. The governor limits how loud the loop
		// gets; the tone limits what it keeps on each pass.
		if (p.loopTone < 1.f) {
			const float cutoff = 100.f * std::pow(200.f, p.loopTone);
			const float alpha = 1.f - std::exp(-2.f * rackmath::PI * cutoff * sampleTime_);
			fbLpL_ += alpha * (outL - fbLpL_);
			fbLpR_ += alpha * (outR - fbLpR_);
			retL = fbLpL_;
			retR = fbLpR_;
		}

		fbHpL_ += hpAlpha_ * (retL - fbHpL_);
		fbHpR_ += hpAlpha_ * (retR - fbHpR_);
		feedbackL_ = retL - fbHpL_;
		feedbackR_ = retR - fbHpR_;

		*outLp = outL;
		*outRp = outR;
	}

	// Exposed so a UI can show how close the loop is to self-oscillation.
	// Below 1 the loop decays, at and above it diverges: a change of sign
	// rather than of degree.
	static float loopGain(const Params& p) {
		return p.feedback * (1.f + 3.f * p.pressure);
	}

	// How hard the governor is currently holding the loop back, as a gain.
	// 1 is not acting. For the stability display, which has to be able to
	// show that the reason the loop stopped growing was the governor rather
	// than the user.
	float governorGain() const { return governorGain_; }

	// The loop's current energy in volts, for the same display.
	float loopEnergy() const { return loopEnv_; }

private:
	static float clampf(float x, float lo, float hi) {
		return x < lo ? lo : (x > hi ? hi : x);
	}

	// The three saturation characters from Collapse Saturator, by its own
	// names and with its own coefficients, so a user who knows the Rack
	// module does not have to relearn them.
	static float saturate(float x, int character) {
		if (character == 1) {
			// EVEN: shifting the operating point adds even harmonics, the
			// way tape does. Subtracting tanh(b) keeps zero in at zero out.
			const float b = 0.35f;
			return std::tanh(x + b) - std::tanh(b);
		}
		if (character >= 2)
			return clampf(x, -1.f, 1.f);   // FULL: hard clip
		return std::tanh(x);               // ODD: symmetric, and the default
	}

	float sampleTime_  = 1.f / 44100.f;
	float hpAlpha_     = 0.f;
	float tiltAlpha_   = 0.f;
	float envAttack_   = 0.f;
	float envRelease_  = 0.f;
	float govAttack_   = 0.f;
	float govRelease_  = 0.f;

	float collapseEnv_ = 1.f;
	float feedbackL_   = 0.f;
	float feedbackR_   = 0.f;
	float fbHpL_       = 0.f;
	float fbHpR_       = 0.f;
	float fbLpL_       = 0.f;
	float fbLpR_       = 0.f;
	float tiltL_       = 0.f;
	float tiltR_       = 0.f;
	float loopEnv_     = 0.f;
	float governorGain_ = 1.f;
};

}  // namespace af
