// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// Checks String Mass Core against output captured before it was rewired to
// call the shared engine.
//
// This replaces tests/equivalence.cpp in the commercial tree, which compared
// the module against the extracted core and proved the port was faithful.
// Once the module CALLS the core that comparison is the core against itself,
// so what guards the sound from here on is a frozen reference rather than a
// second implementation.
//
// It is not an exact comparison, deliberately. The engine uses a polynomial
// sincos rather than libm, measured at 1.22e-07 worst error and 7.3 times
// faster. The reference predates that, so the job here is to show the
// difference is the trig substitution and nothing else.

#include "../src/plugin.hpp"
#include "golden_data.hpp"

#include <cmath>
#include <cstdio>

namespace {
enum { MASS_PARAM, MASS_ATTEN_PARAM, SPREAD_PARAM, SPREAD_ATTEN_PARAM,
       TIMBRE_PARAM, TIMBRE_ATTEN_PARAM, MODE_PARAM, SECTION_PARAM };
enum { VOCT_INPUT };
enum { AUDIO_OUTPUT };

// Output is plus or minus 5 V. A tolerance of 1e-3 V is about -74 dBFS
// relative to that, which is far above the trig error and far below anything
// a structural mistake would produce. A wrong harmonic or a dropped voice
// moves things by volts, not millivolts.
const double TOL = 1e-3;
}  // namespace

extern void init(rack::Plugin* p);

int main() {
	setvbuf(stdout, NULL, _IONBF, 0);
	rack::Plugin* plug = new rack::Plugin;
	plug->path = "."; plug->slug = "amplified-futures";
	init(plug);

	int failures = 0;
	double worst = 0.0;
	const char* worstAt = "";

	std::printf("String Mass Core against frozen reference, tolerance %.0e V\n\n", TOL);

	for (int r = 0; r < af::golden::REF_COUNT; r++) {
		const af::golden::Ref& ref = af::golden::REFS[r];

		rack::engine::Module* m = modelStringMassCore->createModule();
		m->params[MASS_PARAM].setValue(float(ref.voices));
		m->params[SPREAD_PARAM].setValue(ref.spread);
		m->params[TIMBRE_PARAM].setValue(ref.timbre);
		m->params[MODE_PARAM].setValue(float(ref.mode));
		m->params[SECTION_PARAM].setValue(
			ref.sections == 1 ? 0.f : ref.sections == 2 ? 1.f : 2.f);
		m->inputs[VOCT_INPUT].setChannels(1);
		m->inputs[VOCT_INPUT].setVoltage(ref.voct, 0);

		rack::engine::Module::ProcessArgs a;
		a.sampleRate = 48000.f; a.sampleTime = 1.f / 48000.f; a.frame = 0;

		double mine = 0.0;
		for (int n = 0; n < af::golden::WARMUP + af::golden::COUNT; n++) {
			m->process(a); a.frame++;
			if (n < af::golden::WARMUP) continue;
			const double d = std::fabs(
				double(m->outputs[AUDIO_OUTPUT].getVoltage(0)) -
				double(ref.samples[n - af::golden::WARMUP]));
			if (d > mine) mine = d;
		}
		delete m;

		if (mine > worst) { worst = mine; worstAt = ref.name; }
		const bool ok = mine <= TOL;
		if (!ok) failures++;
		std::printf("  %-16s worst |d| %.3g  %s\n", ref.name, mine,
		            ok ? "" : "  FAIL");
	}

	std::printf("\nworst overall %.3g V, in %s\n", worst, worstAt);
	std::printf("%d references, %d failure(s)\n", af::golden::REF_COUNT, failures);
	if (failures == 0)
		std::printf("the rewired module still sounds like the reference\n");
	return failures == 0 ? 0 : 1;
}
