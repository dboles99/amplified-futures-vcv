// Offline smoke test for every module in the plugin.
//
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// The core suites in this directory test DSP headers that were deliberately
// written without a Rack dependency. That covers AF-02 to AF-06 and leaves the
// other fourteen modules untested, because their DSP lives inline in a
// rack::Module subclass and cannot be reached without Rack.
//
// This reaches them the only honest way: link the real libRack, ask each Model
// for a Module exactly as Rack does, and drive process() by hand. Nothing is
// stubbed, so the dsp classes, the maths and the voltage handling are Rack's
// own rather than an approximation that agrees with the code under test.
//
// What it cannot do is tell you a module sounds right. It tells you a module
// does not produce NaN, does not run away, and does not ignore a control.

#include "../src/plugin.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>

namespace {

const float VOLTAGE_LIMIT = 12.f;   // Rack's stated hard limit for any output
const int WARMUP = 64;              // discard: filters and slews start cold
const int RUN = 2048;
const int POLY = 4;      // enough that a per-channel spread control does something

int failures = 0;
int checks = 0;

struct Case {
	const char* name;
	Model** model;
};

Case CASES[] = {
	{"DroneCore",         &modelDroneCore},
	{"DroneClone",        &modelDroneClone},
	{"Send",              &modelSend},
	{"Choke",             &modelChoke},
	{"Pulse",             &modelPulse},
	{"Drift",             &modelDrift},
	{"WallConductor",     &modelWallConductor},
	{"StringMassCore",    &modelStringMassCore},
	{"HarmonicPressure",  &modelHarmonicPressure},
	{"CollapseSaturator", &modelCollapseSaturator},
	{"FeedbackGovernor",  &modelFeedbackGovernor},
	{"MassDriver",        &modelMassDriver},
	{"SitarGrid",         &modelSitarGrid},
	{"SwarmCore",         &modelSwarmCore},
	{"StreetGridClock",   &modelStreetGridClock},
	{"CollapseEG",        &modelCollapseEG},
	{"QuadVCA",           &modelQuadVCA},
	{"Ratchet",           &modelRatchet},
	{"SignalBloc",        &modelSignalBloc},
};

void fail(const std::string& what) {
	std::printf("  FAIL  %s\n", what.c_str());
	failures++;
}

// A signal with content across the spectrum, so a filter or a follower has
// something to act on. Constant DC would let a broken module look fine.
float stimulus(int n, float sr) {
	float t = float(n) / sr;
	float v = 3.f * std::sin(2.f * float(M_PI) * 110.f * t);
	v += 1.5f * std::sin(2.f * float(M_PI) * 1730.f * t);
	v += ((n % 97) < 48) ? 0.4f : -0.4f;      // a little edge, deterministically
	return v;
}

// Audio into a clock input is not a clock. A sequencer fed a sine never
// advances, every step parameter then looks identical, and the harness
// reports two dozen dead controls that are nothing of the sort. So inputs are
// driven according to what they were configured to be.
bool wantsPulses(Module* m, size_t i) {
	if (i >= m->inputInfos.size() || !m->inputInfos[i])
		return false;
	std::string n = m->inputInfos[i]->name;
	for (char& c : n) c = char(std::tolower((unsigned char) c));
	static const char* KEYS[] = {"clock", "clk", "trig", "gate", "sync",
	                             "reset", "kill", "collapse", "choke"};
	for (const char* k : KEYS)
		if (n.find(k) != std::string::npos)
			return true;
	return false;
}

void feedInputs(Module* m, int n, float sr) {
	// ~8 Hz, which is a plausible musical clock at any of the test rates and
	// fast enough to advance a sequencer several times inside one run.
	int period = int(sr / 8.f);
	bool high = period > 0 && (n % period) < (period / 8);

	for (size_t i = 0; i < m->inputs.size(); i++) {
		// Direct member write, not setChannels(). Rack's setChannels() returns
		// early on a port with channels == 0, so a disconnected port stays
		// disconnected and every "polyphonic" test here silently ran on one
		// channel. Modules that use max(1, getChannels()) hid it; Wall
		// Conductor, which sums over getChannels(), is what exposed it.
		m->inputs[i].channels = POLY;
		for (int c = 0; c < POLY; c++) {
			float v = wantsPulses(m, i)
				? (high ? 10.f : 0.f)
				: stimulus(n + int(i) * 13 + c * 5, sr);
			m->inputs[i].setVoltage(v, c);
		}
	}
}

void disconnectInputs(Module* m) {
	for (size_t i = 0; i < m->inputs.size(); i++)
		m->inputs[i].setChannels(0);
}

// Returns a crude signature of everything the module emitted, so two runs can
// be compared. Sum of absolute values is enough to catch "this control does
// nothing at all" without pretending to be a similarity metric.
// Did any output sit at a constant non-zero value for the whole run while the
// input was moving? That is a DC latch, and it is the failure the rest of this
// harness was blind to.
//
// Wall Conductor did exactly this from release until 2026-09-10: its feedback
// bus had no DC blocker, so past a loop gain of 1 it converged on a silent
// rail at up to 4.99 V within ten samples. Finite, inside plus or minus 12 V,
// and therefore passing every check here. Legal output is not the same as
// audio.
// Only audio outputs. A constant non-zero level is correct behaviour for
// plenty of things: CollapseEG's INV is (1 - env) * 10, so at rest it sits at
// a correct and constant 10 V, and Drift's SMOOTH and STEP are CV. Checking
// every output flagged three modules, all of them fine, which is how a useful
// check turns into one nobody reads.
bool isAudioOutput(Module* m, size_t o) {
	if (o >= m->outputInfos.size() || !m->outputInfos[o])
		return false;
	std::string n = m->outputInfos[o]->name;
	for (char& c : n) c = char(std::tolower((unsigned char) c));
	static const char* NOT_AUDIO[] = {
		"env", "inv", "eoc", "end of", "gate", "trig", "clock", "clk",
		"v/oct", "voct", "cv", "pitch", "step", "smooth", "reset", "div",
	};
	for (const char* k : NOT_AUDIO)
		if (n.find(k) != std::string::npos)
			return false;
	return true;
}

bool dcLatched(Module* m, const std::vector<float>& lo, const std::vector<float>& hi) {
	for (size_t o = 0; o < lo.size(); o++) {
		if (!isAudioOutput(m, o))
			continue;
		const float span = hi[o] - lo[o];
		const float level = std::fabs(lo[o]);
		if (span < 1e-6f && level > 0.1f)
			return true;
	}
	return false;
}

double runFor(Module* m, float sr, int samples, bool patched, bool* clean,
              std::vector<float>* loOut, std::vector<float>* hiOut) {
	std::vector<float> lo(m->outputs.size(),  1e30f);
	std::vector<float> hi(m->outputs.size(), -1e30f);
	Module::ProcessArgs args;
	args.sampleRate = sr;
	args.sampleTime = 1.f / sr;
	args.frame = 0;

	double acc = 0.0;
	*clean = true;

	for (int n = 0; n < samples; n++) {
		if (patched)
			feedInputs(m, n, sr);
		m->process(args);
		args.frame++;

		if (n < WARMUP)
			continue;
		for (size_t o = 0; o < m->outputs.size(); o++) {
			int ch = m->outputs[o].getChannels();
			if (ch < 1) ch = 1;
			for (int c = 0; c < ch; c++) {
				float v = m->outputs[o].getVoltage(c);
				if (!std::isfinite(v)) { *clean = false; return acc; }
				if (std::fabs(v) > VOLTAGE_LIMIT) { *clean = false; return acc; }
				if (c == 0) { if (v < lo[o]) lo[o] = v; if (v > hi[o]) hi[o] = v; }
				acc += std::fabs(double(v));
			}
		}
	}
	if (loOut) *loOut = lo;
	if (hiOut) *hiOut = hi;
	return acc;
}

void resetToDefaults(Module* m) {
	for (size_t i = 0; i < m->params.size(); i++) {
		ParamQuantity* pq = m->paramQuantities[i];
		m->params[i].setValue(pq ? pq->defaultValue : 0.f);
	}
	disconnectInputs(m);
	for (size_t o = 0; o < m->outputs.size(); o++) {
		m->outputs[o].setChannels(1);
		m->outputs[o].setVoltage(0.f);
	}
}

void testModule(const Case& cs) {
	std::printf("== %s\n", cs.name);
	Model* model = *cs.model;
	if (!model) { fail(std::string(cs.name) + ": model is null"); return; }

	const float RATES[] = {44100.f, 48000.f, 96000.f};

	for (float sr : RATES) {
		Module* m = model->createModule();
		if (!m) { fail(std::string(cs.name) + ": createModule returned null"); return; }

		// 1. defaults, nothing patched. Must not run away on its own.
		resetToDefaults(m);
		bool clean = true;
		runFor(m, sr, RUN, false, &clean, nullptr, nullptr);
		checks++;
		if (!clean)
			fail(std::string(cs.name) + " @" + std::to_string(int(sr)) +
			     ": unpatched output is not finite or exceeds " +
			     std::to_string(int(VOLTAGE_LIMIT)) + "V");

		// 2. everything patched, every param at each extreme. This is where a
		//    feedback path that only misbehaves at the top of its range shows up.
		for (int extreme = 0; extreme < 2; extreme++) {
			resetToDefaults(m);
			for (size_t i = 0; i < m->params.size(); i++) {
				ParamQuantity* pq = m->paramQuantities[i];
				if (!pq) continue;
				m->params[i].setValue(extreme ? pq->maxValue : pq->minValue);
			}
			clean = true;
			std::vector<float> lo, hi;
			runFor(m, sr, RUN, true, &clean, &lo, &hi);
			checks++;
			if (!clean)
				fail(std::string(cs.name) + " @" + std::to_string(int(sr)) +
				     ": all params at " + (extreme ? "max" : "min") +
				     " produced a non-finite or out-of-range output");

			// The input is moving throughout. An output that does not move,
			// and is not at zero, has latched.
			checks++;
			if (clean && dcLatched(m, lo, hi))
				fail(std::string(cs.name) + " @" + std::to_string(int(sr)) +
				     ": all params at " + (extreme ? "max" : "min") +
				     " left an output stuck at a constant non-zero value");
		}

		delete m;
	}

	// There was a third check here, comparing each parameter at both extremes
	// to find controls that do nothing. It was removed rather than fixed.
	//
	// It could not distinguish a genuinely unwired control from one that needs
	// the module to be in some state the harness has no way to reach. Driving
	// a sequencer with a clock instead of audio cut the noise a long way, but
	// it still reported every attenuverter on eight modules and forty
	// parameters on Sitar Grid, none of which is dead. A check that is wrong
	// sixty times teaches you to skip reading it, which then costs you the one
	// time it was right.
	//
	// Finding real dead controls needs a per-module fixture describing what
	// makes each parameter observable. That is worth writing, and it is not
	// this file. The 2026-08-03 audit found six dead controls by launching the
	// thing and looking at it, which remains the method that works.
}

}  // namespace

// Defined in src/plugin.cpp. Rack normally calls this. Here we must, because
// module constructors reach for pluginInstance to resolve asset paths and
// abort on a null one, which is how SwarmCore took the whole run down.
extern void init(rack::Plugin* p);

int main() {
	// Unbuffered, because this harness exists to survive things going wrong.
	// Block-buffered stdout through a pipe loses everything printed before a
	// crash, which turns "it died in SitarGrid" into "it died".
	setvbuf(stdout, NULL, _IONBF, 0);

	rack::Plugin* p = new rack::Plugin;
	p->path = ".";          // the harness runs from the plugin root
	p->slug = "amplified-futures";
	init(p);

	std::printf("Offline smoke test, %zu modules, 3 sample rates\n\n",
	            sizeof(CASES) / sizeof(CASES[0]));

	for (const Case& cs : CASES)
		testModule(cs);

	std::printf("\n%d checks, %d failure(s)\n", checks, failures);
	if (failures == 0)
		std::printf("all pass\n");
	return failures == 0 ? 0 : 1;
}
