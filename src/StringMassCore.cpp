// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include "dsp/MassEngine.hpp"

// ============================================================
// STRING MASS CORE — 16-voice harmonic mass oscillator
//
// Design: Amplified Futures (steel finish, 16 HP)
//   MASS:    1–16 internal voices  (snap, CV-able)
//   SPREAD:  per-voice detune 0–50 cents
//   TIMBRE:  sine → third-bridge harmonic stack
//   MODE:    UNIS / HARM / JUST / MICRO (snap)
//   SECTION: 1 / 2 / 4 harmonic sections (snap, HARM mode only)
//
// Modes:
//   UNIS  — all M voices at fundamental, symmetric ±SPREAD cents
//   HARM  — M voices divided into SECTION groups; each group at
//            an odd harmonic (1,3,5,7→octave-reduced), within-
//            group spread creates dense beating layers
//   JUST  — M voices across JI chromatic ratios + narrow SPREAD
//   MICRO — all voices at fundamental with per-voice slow vibrato
//            at different rates; produces organic shimmer
//
// Amplitude: 1/√M normalisation → ~5V output; tanh soft-clip
// Polyphonic: one mass per V/OCT channel, output poly matches
// ============================================================

// The tuning tables and the engine itself now live in dsp/MassEngine.hpp,
// shared with the commercial plug-in so the two renderers cannot drift.

struct StringMassCore : Module {
	enum ParamId {
		MASS_PARAM,
		MASS_ATTEN_PARAM,
		SPREAD_PARAM,
		SPREAD_ATTEN_PARAM,
		TIMBRE_PARAM,
		TIMBRE_ATTEN_PARAM,
		MODE_PARAM,
		SECTION_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		MASS_CV_INPUT,
		SPREAD_CV_INPUT,
		TIMBRE_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIO_OUTPUT,
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	// One mass per polyphonic channel. The engine owns its own phase state
	// and staggers it on construction, which is what the loop in this
	// constructor used to do by hand.
	af::MassEngine engines[16];
	float engineSr = 0.f;

	StringMassCore() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(MASS_PARAM,           1.f,  16.f, 4.f,  "Mass",    " voices");
		configParam(MASS_ATTEN_PARAM,    -1.f,   1.f, 0.f,  "Mass attenuverter");
		configParam(SPREAD_PARAM,         0.f,   1.f, 0.3f, "Spread",  "%", 0.f, 100.f);
		configParam(SPREAD_ATTEN_PARAM,  -1.f,   1.f, 0.f,  "Spread attenuverter");
		configParam(TIMBRE_PARAM,         0.f,   1.f, 0.3f, "Timbre",  "%", 0.f, 100.f);
		configParam(TIMBRE_ATTEN_PARAM,  -1.f,   1.f, 0.f,  "Timbre attenuverter");
		configParam(MODE_PARAM,           0.f,   3.f, 1.f,  "Mode");
		configParam(SECTION_PARAM,        0.f,   2.f, 1.f,  "Sections");

		getParamQuantity(MASS_PARAM)->snapEnabled    = true;
		getParamQuantity(MODE_PARAM)->snapEnabled    = true;
		getParamQuantity(SECTION_PARAM)->snapEnabled = true;
		getParamQuantity(MODE_PARAM)->description    = "0=UNIS  1=HARM  2=JUST  3=MICRO";
		getParamQuantity(SECTION_PARAM)->description = "0=1 section  1=2 sections  2=4 sections (HARM mode)";

		configInput(VOCT_INPUT,      "V/oct pitch (poly)");
		configInput(MASS_CV_INPUT,   "Mass CV");
		configInput(SPREAD_CV_INPUT, "Spread CV");
		configInput(TIMBRE_CV_INPUT, "Timbre CV");
		configOutput(AUDIO_OUTPUT,   "Audio mass (poly)");
		configOutput(VOCT_OUTPUT,    "V/oct thru (poly)");

		// Bypass passes audio through rather than muting it. Without this, bypassing
		// the module drops its outputs to zero and the patch goes quiet.
		configBypass(VOCT_INPUT, VOCT_OUTPUT);

	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float val = params[param].getValue();
		if (inputs[cv].isConnected())
			val += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(val, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		if (args.sampleRate != engineSr) {
			engineSr = args.sampleRate;
			for (auto& e : engines)
				e.setSampleRate(engineSr);
		}

		const int polyCh = std::max(1, inputs[VOCT_INPUT].getChannels());
		outputs[AUDIO_OUTPUT].setChannels(polyCh);
		outputs[VOCT_OUTPUT].setChannels(polyCh);

		af::MassEngine::Params p;
		p.voices = (int) std::round(modp(MASS_PARAM, MASS_ATTEN_PARAM, MASS_CV_INPUT, 1.f, 16.f));
		p.spread = modp(SPREAD_PARAM, SPREAD_ATTEN_PARAM, SPREAD_CV_INPUT, 0.f, 1.f);
		p.timbre = modp(TIMBRE_PARAM, TIMBRE_ATTEN_PARAM, TIMBRE_CV_INPUT, 0.f, 1.f);
		p.mode   = clamp((int) std::round(params[MODE_PARAM].getValue()), 0, 3);

		const int secParam = clamp((int) std::round(params[SECTION_PARAM].getValue()), 0, 2);
		p.sections = (secParam == 0) ? 1 : (secParam == 1) ? 2 : 4;

		// The engine clamps voices and sections itself, so this does not.
		for (int c = 0; c < polyCh; c++) {
			const float voct = inputs[VOCT_INPUT].getVoltage(c);
			outputs[AUDIO_OUTPUT].setVoltage(engines[c].process(voct, p), c);
			outputs[VOCT_OUTPUT].setVoltage(voct, c);
		}
	}
};

// ============================================================
// WIDGET  (16 HP)
// ============================================================

struct StringMassCoreWidget : ModuleWidget {
	StringMassCoreWidget(StringMassCore* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/StringMassCore.svg")));

		// 16HP screws

		// ── Row 1: MASS (L) | SPREAD (R) ──────────────────────────
		// L=15mm, satellite at L+8=23mm
		// R=62mm, satellite at R+8=70mm

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 38.f)), module, StringMassCore::MASS_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(23.f, 31.f)), module, StringMassCore::MASS_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(23.f, 45.f)), module, StringMassCore::MASS_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(62.f, 38.f)), module, StringMassCore::SPREAD_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(70.f, 31.f)), module, StringMassCore::SPREAD_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(70.f, 45.f)), module, StringMassCore::SPREAD_CV_INPUT));

		// ── Row 2: TIMBRE (L) | MODE (R, discrete) ────────────────

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 68.f)), module, StringMassCore::TIMBRE_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(23.f, 61.f)), module, StringMassCore::TIMBRE_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(23.f, 75.f)), module, StringMassCore::TIMBRE_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(62.f, 68.f)), module, StringMassCore::MODE_PARAM));

		// ── Row 3: SECTION (centre, discrete) ─────────────────────

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(40.f, 92.f)), module, StringMassCore::SECTION_PARAM));

		// ── Row 4: IO ──────────────────────────────────────────────
		addInput(createInputCentered<AFPortIn>( mm2px(Vec(15.f, 110.f)), module, StringMassCore::VOCT_INPUT));
		addOutput(createOutputCentered<AFPortOut>(mm2px(Vec(40.f, 110.f)), module, StringMassCore::VOCT_OUTPUT));
		addOutput(createOutputCentered<AFPortOut>(mm2px(Vec(65.f, 110.f)), module, StringMassCore::AUDIO_OUTPUT));
	}
};

Model* modelStringMassCore = createModel<StringMassCore, StringMassCoreWidget>("StringMassCore");
