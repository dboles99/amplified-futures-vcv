// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include "dsp/AfDrift.hpp"

// ============================================================
// HARMONIC PRESSURE — harmonic series pitch CV generator
//
// Design: Amplified Futures (steel finish, 14 HP)
//   PITCH:      root pitch offset (-2 to +2 oct)  [CV + atten]
//   SPREAD:     per-partial drift depth            [CV + atten]
//   PARTIAL:    first partial index (1–16)         [snap]
//   COUNT:      number of partials to output (1–16)[snap]
//   TUNING:     JUST / EQUAL / DRIFT               [snap 3-pos]
//   DRIFT RATE: drift oscillator speed (0–4 Hz)
//   DRIFT COH:  0 = whole stack transposes together,
//               1 = partials drift independently (chorus)
//
// Outputs polyphonic V/OCT — COUNT channels, each a harmonic
// partial of the root:  partial n  →  root + log2(n) octaves
//
// JUST:  exact harmonic series ratios (pure JI)
// EQUAL: each partial rounded to nearest 12-TET semitone
//
// Every mode now carries a live per-partial drift on top of its
// base tuning (af::tuning::Drift, see dsp/AfDrift.hpp). SPREAD sets
// the depth in cents, RATE sets how fast it moves, and COHERENCE
// decides whether that movement is shared (transposition) or
// independent per partial (chorus/ensemble spread). At RATE 0 the
// drift oscillator's phase is frozen wherever reset() staggered it,
// so with SPREAD at 0 (the factory default) it contributes exactly
// 0 cents — existing patches with no SPREAD are unaffected. A patch
// that already used a nonzero SPREAD will hear a different (but
// similarly small, static) per-partial colour once DRIFT RATE/
// COHERENCE take their new-param defaults, because the offset is no
// longer the old sin(n * golden angle) formula.
// ============================================================

struct HarmonicPressure : Module {
	enum ParamId {
		PITCH_PARAM,
		PITCH_ATTEN_PARAM,
		SPREAD_PARAM,
		SPREAD_ATTEN_PARAM,
		PARTIAL_PARAM,
		COUNT_PARAM,
		TUNING_PARAM,
		// Appended 2026-09-05 (Task 4, pitch precision work). VCV Rack
		// serialises params by position — never insert above this line,
		// it would silently corrupt every saved patch.
		DRIFT_RATE_PARAM,
		DRIFT_COHERENCE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		PITCH_CV_INPUT,
		SPREAD_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	af::tuning::Drift drift_;

	HarmonicPressure() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PITCH_PARAM,        -2.f,  2.f,  0.f,  "Pitch",   " Oct");
		configParam(PITCH_ATTEN_PARAM,  -1.f,  1.f,  0.f,  "Pitch attenuverter");
		configParam(SPREAD_PARAM,        0.f,  1.f,  0.f,  "Spread",  "%", 0.f, 100.f);
		configParam(SPREAD_ATTEN_PARAM, -1.f,  1.f,  0.f,  "Spread attenuverter");
		configParam(PARTIAL_PARAM,       1.f,  16.f, 1.f,  "First partial");
		configParam(COUNT_PARAM,         1.f,  16.f, 8.f,  "Partial count");
		configParam(TUNING_PARAM,        0.f,  2.f,  0.f,  "Tuning mode");
		configParam(DRIFT_RATE_PARAM,    0.f,  4.f,  0.f,  "Drift rate", " Hz");
		// 0 = the whole stack transposes together; 1 = independent per partial.
		configParam(DRIFT_COHERENCE_PARAM, 0.f, 1.f, 1.f, "Drift coherence");
		drift_.reset(16, 0x5EEDu);

		getParamQuantity(PARTIAL_PARAM)->snapEnabled = true;
		getParamQuantity(COUNT_PARAM)->snapEnabled   = true;
		getParamQuantity(TUNING_PARAM)->snapEnabled  = true;

		getParamQuantity(PARTIAL_PARAM)->description = "Starting harmonic partial (1 = fundamental)";
		getParamQuantity(COUNT_PARAM)->description   = "Number of partials to output as poly channels";
		// Drift (SPREAD/RATE/COHERENCE) now applies in every mode, not only
		// this switch's third position — DRIFT and JUST share the same
		// harmonic maths and differ only in name; only EQUAL is distinct.
		getParamQuantity(TUNING_PARAM)->description  = "0=JUST (pure JI)  1=EQUAL (12-TET)  2=DRIFT (same maths as JUST)";

		configInput(VOCT_INPUT,      "Root V/oct");
		configInput(PITCH_CV_INPUT,  "Pitch offset CV");
		configInput(SPREAD_CV_INPUT, "Spread CV");
		configOutput(VOCT_OUTPUT,    "Harmonic series V/oct (poly)");
	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		// Root pitch: VOCT input + PITCH knob offset
		float root = inputs[VOCT_INPUT].getVoltage();   // 0V if disconnected
		float pitchOffset = params[PITCH_PARAM].getValue();
		if (inputs[PITCH_CV_INPUT].isConnected())
			pitchOffset += params[PITCH_ATTEN_PARAM].getValue()
			             * inputs[PITCH_CV_INPUT].getVoltage() / 5.f;
		root += clamp(pitchOffset, -4.f, 4.f);

		float spread = modp(SPREAD_PARAM, SPREAD_ATTEN_PARAM, SPREAD_CV_INPUT, 0.f, 1.f);
		float spreadCents = spread * 20.f;   // max ±20 cents per partial

		int first = clamp((int)std::round(params[PARTIAL_PARAM].getValue()), 1, 16);
		int count = clamp((int)std::round(params[COUNT_PARAM].getValue()),   1, 16);
		// Clamp so we don't exceed partial 32 (keeps V/OCT in sane range)
		count = std::min(count, 33 - first);

		int tuning = clamp((int)std::round(params[TUNING_PARAM].getValue()), 0, 2);

		drift_.setRate(params[DRIFT_RATE_PARAM].getValue());
		drift_.setDepth(spreadCents);
		drift_.setCoherence(params[DRIFT_COHERENCE_PARAM].getValue());
		drift_.process(args.sampleTime);

		outputs[VOCT_OUTPUT].setChannels(count);

		for (int i = 0; i < count; i++) {
			int n = first + i;   // partial index (1 = fundamental)

			// Exact harmonic series: partial n is at log2(n) octaves above root
			float voct = root + std::log2(float(n));

			if (tuning == 1) {
				// EQUAL: quantise to nearest 12-TET semitone
				voct = std::round(voct * 12.f) / 12.f;
			}

			// Was a static sin(n * golden angle) offset - an ensemble colour
			// frozen in time. Drift makes it move, and coherence decides
			// whether the stack transposes or spreads.
			voct += drift_.centsFor(n - first) / 1200.f;

			outputs[VOCT_OUTPUT].setVoltage(voct, i);
		}
	}
};

// ============================================================
// WIDGET  (14 HP)
// ============================================================

struct HarmonicPressureWidget : ModuleWidget {
	HarmonicPressureWidget(HarmonicPressure* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/HarmonicPressure.svg")));

		// 14HP screws

		// ── Row 1: PITCH (L) | SPREAD (R) ─────────────────────────
		// L=15mm  sat=23mm (+8)   R=55mm  sat=63mm (+8)

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 40.f)), module, HarmonicPressure::PITCH_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(23.f, 33.f)), module, HarmonicPressure::PITCH_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(23.f, 47.f)), module, HarmonicPressure::PITCH_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.f, 40.f)), module, HarmonicPressure::SPREAD_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(63.f, 33.f)), module, HarmonicPressure::SPREAD_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(63.f, 47.f)), module, HarmonicPressure::SPREAD_CV_INPUT));

		// ── Row 2: PARTIAL (L) | COUNT (R) — discrete snap ────────

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 72.f)), module, HarmonicPressure::PARTIAL_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.f, 72.f)), module, HarmonicPressure::COUNT_PARAM));

		// ── Row 3: RATE (L) | TUNING (centre, discrete snap) | COH (R) ──
		// Row 2→3 leaves only ~11mm of clear panel above the TUNING
		// label (divider at y=77.2mm, TUNING's own label starts ~88.4mm)
		// — not enough room for the atten+CV satellite pattern used in
		// Row 1 without colliding with the tuning-mode arc labels or the
		// Row 4 I/O jacks below. Bare knobs only; no CV input added here.

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 96.f)), module, HarmonicPressure::DRIFT_RATE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(35.f, 96.f)), module, HarmonicPressure::TUNING_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.f, 96.f)), module, HarmonicPressure::DRIFT_COHERENCE_PARAM));

		// ── Row 4: IO ──────────────────────────────────────────────
		addInput(createInputCentered<AFPortIn>( mm2px(Vec(15.f, 114.f)), module, HarmonicPressure::VOCT_INPUT));
		addOutput(createOutputCentered<AFPortOut>(mm2px(Vec(55.f, 114.f)), module, HarmonicPressure::VOCT_OUTPUT));
	}
};

Model* modelHarmonicPressure = createModel<HarmonicPressure, HarmonicPressureWidget>("HarmonicPressure");
