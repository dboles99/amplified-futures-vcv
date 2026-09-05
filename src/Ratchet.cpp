// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include "dsp/RatchetCore.hpp"

// ============================================================
// RATCHET (AF-03) — 8 HP burst generator
//
// One trigger in, a burst out. The no-wave stutter.
//
//   PARAMS:  COUNT (+ atten) · SPREAD · PROB
//   INPUTS:  TRIG · COUNT CV
//   OUTPUTS: TRIG · END
//
// No RATE knob: the burst fills the interval between the two most recent
// input triggers, so feeding Street Grid Clock into it subdivides that clock
// and it stays in time on its own. The first trigger passes through alone -
// the module has not seen an interval yet, and guessing one would put the
// first burst at the wrong tempo.
//
// RatchetCore is unit-tested offline in tests/test_ratchet_core.cpp.
// ============================================================

struct AFOrangeLightRatchet : GrayModuleLightWidget {
	AFOrangeLightRatchet() { addBaseColor(nvgRGB(0xFF, 0x4A, 0x0E)); }
};

struct Ratchet : Module {
	enum ParamId {
		COUNT_PARAM,
		COUNT_ATTEN_PARAM,
		SPREAD_PARAM,
		PROB_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIG_INPUT,
		COUNT_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		TRIG_OUTPUT,
		END_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		BURST_LIGHT,
		LIGHTS_LEN
	};

	RatchetCore core;
	dsp::SchmittTrigger trigIn;

	Ratchet() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(COUNT_PARAM, 1.f, 8.f, 2.f, "Repeats");
		getParamQuantity(COUNT_PARAM)->snapEnabled = true;
		configParam(COUNT_ATTEN_PARAM, -1.f, 1.f, 0.f, "Repeats CV amount");
		configParam(SPREAD_PARAM, -1.f, 1.f, 0.f, "Spread");
		configParam(PROB_PARAM, 0.f, 1.f, 1.f, "Repeat probability", "%", 0.f, 100.f);

		configInput(TRIG_INPUT, "Trigger");
		configInput(COUNT_CV_INPUT, "Repeats CV");
		configOutput(TRIG_OUTPUT, "Burst");
		configOutput(END_OUTPUT, "End of burst");

		core.setSampleRate(APP->engine->getSampleRate());
	}

	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		core.setSampleRate(e.sampleRate);
	}

	void onReset(const ResetEvent& e) override {
		Module::onReset(e);
		core.reset();
	}

	void process(const ProcessArgs& args) override {
		// COUNT is an integer, so it gets its own CV maths rather than modp():
		// full CV swing covers the whole 1-8 span, and the result is rounded
		// once at the end so the knob and the CV cannot disagree by a step.
		float count = params[COUNT_PARAM].getValue();
		if (inputs[COUNT_CV_INPUT].isConnected()) {
			const float cv = inputs[COUNT_CV_INPUT].getVoltage() / 10.f;
			count += params[COUNT_ATTEN_PARAM].getValue() * cv * 7.f;
		}
		const int n = int(std::round(clamp(count, 1.f, 8.f)));

		const bool edge = trigIn.process(inputs[TRIG_INPUT].getVoltage(), 0.1f, 1.f);
		RatchetCore::Out out = core.process(edge, n,
		                                    params[SPREAD_PARAM].getValue(),
		                                    params[PROB_PARAM].getValue());

		outputs[TRIG_OUTPUT].setVoltage(out.trig ? 10.f : 0.f);
		outputs[END_OUTPUT].setVoltage(out.end ? 10.f : 0.f);

		// A 1 ms pulse is invisible at frame rate, so the light is smoothed.
		// It shows the burst, which is the thing you want to see on a ratchet.
		lights[BURST_LIGHT].setBrightnessSmooth(out.trig ? 1.f : 0.f, args.sampleTime, 20.f);
	}
};

struct RatchetWidget : ModuleWidget {
	RatchetWidget(Ratchet* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Ratchet.svg")));


		// 8 HP = 40.64 mm. Clearances were computed before the panel was drawn
		// - see the comment block in res/Ratchet.svg for the full arithmetic.
		// Tightest vertical pair: the ATT/CV label row to the CV port, 4.2 px.
		const float xc = 20.32f;   // centre column

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xc,     24.384f)), module, Ratchet::COUNT_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec( 8.467f, 40.640f)), module, Ratchet::COUNT_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(32.173f, 40.640f)), module, Ratchet::COUNT_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xc,     58.928f)), module, Ratchet::SPREAD_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xc,     75.862f)), module, Ratchet::PROB_PARAM));

		addChild(createLightCentered<SmallLight<AFOrangeLightRatchet>>(mm2px(Vec(32.850f, 24.384f)), module, Ratchet::BURST_LIGHT));

		addInput(createInputCentered<AFPortIn>(  mm2px(Vec(xc,      98.891f)), module, Ratchet::TRIG_INPUT));
		addOutput(createOutputCentered<AFPortOut>(mm2px(Vec(13.547f, 116.501f)), module, Ratchet::TRIG_OUTPUT));
		addOutput(createOutputCentered<AFPortOut>(mm2px(Vec(27.093f, 116.501f)), module, Ratchet::END_OUTPUT));
	}
};

Model* modelRatchet = createModel<Ratchet, RatchetWidget>("Ratchet");
