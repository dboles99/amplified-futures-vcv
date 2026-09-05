// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"

// ============================================================
// COLLAPSE SATURATOR — stereo drive/saturation with collapse
//
// Design: Amplified Futures (steel finish, 12 HP)
//   DRIVE:    pre-gain ×1–×10                  [CV + atten]
//   BUZZ:     saturation character 3-way switch
//             ODD  — symmetric tanh (tube-like, odd harmonics)
//             EVEN — asymmetric (tape-like, even harmonics)
//             FULL — hard clip (fuzz, full harmonic spectrum)
//   RECOVERY: collapse recovery time            [CV + atten]
//   COLLAPSE: gate input → instantly maxes DRIVE + BUZZ
//             then recovers to set values over RECOVERY time
//   SIDECHAIN: adds drive proportional to its level
//              (patch PULSE or DroneClone for transient colour)
//   Stereo IN/OUT + V/OCT thru
// ============================================================

struct CollapseSaturator : Module {
	enum ParamId {
		DRIVE_PARAM,
		DRIVE_ATTEN_PARAM,
		BUZZ_PARAM,
		RECOVERY_PARAM,
		RECOVERY_ATTEN_PARAM,
		// Appended - never inserted above. Rack serialises params by position,
		// so an insert would silently remap every saved patch.
		LEVEL_PARAM,
		MIX_PARAM,
		SC_AMT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		IN_L_INPUT,
		IN_R_INPUT,
		DRIVE_CV_INPUT,
		RECOVERY_CV_INPUT,
		COLLAPSE_INPUT,
		SIDECHAIN_INPUT,
		VOCT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float collapseEnv = 0.f;

	CollapseSaturator() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(DRIVE_PARAM,          0.f, 1.f, 0.3f, "Drive",    "%", 0.f, 100.f);
		configParam(DRIVE_ATTEN_PARAM,   -1.f, 1.f, 0.f,  "Drive attenuverter");
		configSwitch(BUZZ_PARAM,          0.f, 2.f, 0.f,  "Buzz character",
		             {"ODD — symmetric tanh", "EVEN — asymmetric (tape)", "FULL — hard clip"});
		configParam(RECOVERY_PARAM,       0.f, 1.f, 0.3f, "Recovery", "%", 0.f, 100.f);
		configParam(RECOVERY_ATTEN_PARAM,-1.f, 1.f, 0.f,  "Recovery attenuverter");

		// Defaults reproduce the previous behaviour exactly: unity out, fully
		// wet, and the sidechain depth that used to be hard-coded at 0.5.
		configParam(LEVEL_PARAM,  0.f, 2.f, 1.f, "Output level", "x");
		configParam(MIX_PARAM,    0.f, 1.f, 1.f, "Dry / wet", "%", 0.f, 100.f);
		configParam(SC_AMT_PARAM, 0.f, 1.f, 0.5f, "Sidechain amount", "%", 0.f, 100.f);

		configInput(IN_L_INPUT,       "Left");
		configInput(IN_R_INPUT,       "Right");
		configInput(DRIVE_CV_INPUT,   "Drive CV");
		configInput(RECOVERY_CV_INPUT,"Recovery CV");
		configInput(COLLAPSE_INPUT,   "Collapse gate");
		configInput(SIDECHAIN_INPUT,  "Sidechain (boosts drive by signal level)");
		configInput(VOCT_INPUT,       "V/oct (thru)");
		configOutput(OUT_L_OUTPUT,    "Left");
		configOutput(OUT_R_OUTPUT,    "Right");
		configOutput(VOCT_OUTPUT,     "V/oct (thru)");
	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		float drive    = modp(DRIVE_PARAM,    DRIVE_ATTEN_PARAM,    DRIVE_CV_INPUT,    0.f, 1.f);
		float recovery = modp(RECOVERY_PARAM, RECOVERY_ATTEN_PARAM, RECOVERY_CV_INPUT, 0.f, 1.f);
		int   buzz     = clamp((int)std::round(params[BUZZ_PARAM].getValue()), 0, 2);

		// Sidechain: boosts drive proportional to its amplitude
		if (inputs[SIDECHAIN_INPUT].isConnected()) {
			float sc = std::abs(inputs[SIDECHAIN_INPUT].getVoltage()) / 5.f;
			drive = clamp(drive + sc * params[SC_AMT_PARAM].getValue(), 0.f, 1.f);
		}

		// Collapse: gate fires → ramp to full drive + hard clip, then recover
		bool active = inputs[COLLAPSE_INPUT].getVoltage() >= 1.f;
		float tau   = active ? 0.001f : (0.01f + recovery * 2.f);
		collapseEnv += ((active ? 1.f : 0.f) - collapseEnv)
		             * (1.f - std::exp(-args.sampleTime / tau));

		float effDrive = drive + collapseEnv * (1.f - drive);
		float preGain  = 1.f + effDrive * 9.f;   // ×1 to ×10

		for (int ch = 0; ch < 2; ch++) {
			int inId  = (ch == 0) ? IN_L_INPUT  : IN_R_INPUT;
			int outId = (ch == 0) ? OUT_L_OUTPUT : OUT_R_OUTPUT;

			const float dry = inputs[inId].getVoltage();
			float x = dry / 5.f * preGain;

			float sat;
			if (buzz == 0) {
				// ODD: smooth symmetric tanh — odd harmonics only (tube)
				sat = std::tanh(x);
			} else if (buzz == 1) {
				// EVEN: asymmetric — shift operating point adds even harmonics (tape)
				// tanh(x+b) - tanh(b) keeps zero-input → zero-output
				float b = 0.35f;
				sat = std::tanh(x + b) - std::tanh(b);
			} else {
				// FULL: hard clip — full harmonic spectrum (fuzz)
				sat = clamp(x, -1.f, 1.f);
			}

			// Collapse blends toward hard clip regardless of BUZZ mode
			if (collapseEnv > 0.f && buzz < 2) {
				float hard = clamp(x, -1.f, 1.f);
				sat = sat + (hard - sat) * collapseEnv;
			}

			// Parallel blend, then output trim. preGain reaches x10, so
			// without a trim the only way to hear more drive was more level.
			const float wet = sat * 5.f;
			const float mix = params[MIX_PARAM].getValue();
			const float out = (dry * (1.f - mix) + wet * mix)
			                * params[LEVEL_PARAM].getValue();
			outputs[outId].setVoltage(clamp(out, -11.7f, 11.7f));
		}

		outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage());
	}
};

// ============================================================
// WIDGET  (12 HP — Drift-style horizontal satellite layout)
// ============================================================

struct CollapseSaturatorWidget : ModuleWidget {
	CollapseSaturatorWidget(CollapseSaturator* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/CollapseSat.svg")));


		// ── Row 1: DRIVE (L) | BUZZ switch (R) ───────────────────
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(20.32f,24.45f)), module, CollapseSaturator::DRIVE_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(12.32f,36.45f)), module, CollapseSaturator::DRIVE_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(28.32f,36.45f)), module, CollapseSaturator::DRIVE_CV_INPUT));

		addParam(createParamCentered<CKSSThree>(          mm2px(Vec(60.96f,28.45f)), module, CollapseSaturator::BUZZ_PARAM));

		// ── Row 2: RECOVERY (L) | COLLAPSE + SIDECHAIN (R) ───────
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(20.32f,56.45f)), module, CollapseSaturator::RECOVERY_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(12.32f,68.45f)), module, CollapseSaturator::RECOVERY_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(28.32f,68.45f)), module, CollapseSaturator::RECOVERY_CV_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.96f,54.45f)), module, CollapseSaturator::COLLAPSE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(60.96f,70.45f)), module, CollapseSaturator::SIDECHAIN_INPUT));

		// Middle column: the 16 HP widening left it empty, and these three are
		// the controls the module was actually missing rather than filler.
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(40.64f, 24.45f)), module, CollapseSaturator::LEVEL_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(40.64f, 56.45f)), module, CollapseSaturator::MIX_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(49.50f, 70.45f)), module, CollapseSaturator::SC_AMT_PARAM));

		// ── Row 3+4: Stereo IO + V/OCT thru ──────────────────────
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(20.32f,90.45f)), module, CollapseSaturator::IN_L_INPUT));
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(40.64f,90.45f)), module, CollapseSaturator::IN_R_INPUT));
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(60.96f,90.45f)), module, CollapseSaturator::VOCT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32f,102.5f)), module, CollapseSaturator::OUT_L_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(40.64f,102.5f)), module, CollapseSaturator::OUT_R_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(60.96f,102.5f)), module, CollapseSaturator::VOCT_OUTPUT));
	}
};

Model* modelCollapseSaturator = createModel<CollapseSaturator, CollapseSaturatorWidget>("CollapseSat");
