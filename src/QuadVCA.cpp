// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include "dsp/VcaCore.hpp"

// ============================================================
// QUAD VCA (AF-05) — 12 HP four-channel VCA and mixer
//
// The series had no VCA: nothing could apply an envelope to a signal.
// This is that module, and because the inputs are normalled down the
// chain it doubles as a small mixer.
//
//   PARAMS:  LEVEL 1-4 · PRESSURE · LIN/EXP
//   INPUTS:  IN 1-4 · CV 1-4
//   OUTPUTS: OUT 1-4 · MIX
//
// PRESSURE saturates the summed output and is bit-transparent at 0.
// VcaCore is unit-tested offline in tests/test_vca_core.cpp.
// ============================================================

struct QuadVCA : Module {
	enum ParamId {
		LEVEL1_PARAM, LEVEL2_PARAM, LEVEL3_PARAM, LEVEL4_PARAM,
		PRESSURE_PARAM,
		CURVE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		IN1_INPUT, IN2_INPUT, IN3_INPUT, IN4_INPUT,
		CV1_INPUT, CV2_INPUT, CV3_INPUT, CV4_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT1_OUTPUT, OUT2_OUTPUT, OUT3_OUTPUT, OUT4_OUTPUT,
		MIX_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	QuadVCA() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int i = 0; i < 4; i++) {
			configParam(LEVEL1_PARAM + i, 0.f, 1.f, 1.f,
				string::f("Channel %d level", i + 1), "%", 0.f, 100.f);
			configInput(IN1_INPUT + i, string::f("Channel %d", i + 1));
			configInput(CV1_INPUT + i, string::f("Channel %d CV", i + 1));
			configOutput(OUT1_OUTPUT + i, string::f("Channel %d", i + 1));
		}
		configParam(PRESSURE_PARAM, 0.f, 1.f, 0.f, "Pressure", "%", 0.f, 100.f);
		configSwitch(CURVE_PARAM, 0.f, 1.f, 0.f, "Response", {"Linear", "Exponential"});
		configOutput(MIX_OUTPUT, "Mix");
	}

	void process(const ProcessArgs& args) override {
		const bool expo = params[CURVE_PARAM].getValue() > 0.5f;
		const float pressure = params[PRESSURE_PARAM].getValue();

		float mix = 0.f;
		float carried = 0.f;   // normalling: last patched input travels down

		for (int i = 0; i < 4; i++) {
			// Inputs are normalled down the chain, so patching only channel 1
			// feeds all four and the module works as a mixer with per-channel
			// level. An unpatched first channel means silence, not noise.
			if (inputs[IN1_INPUT + i].isConnected())
				carried = inputs[IN1_INPUT + i].getVoltage();
			const float in = carried;

			// Knob sets the ceiling; CV scales within it. Unpatched CV leaves
			// the knob in sole control rather than muting the channel.
			float amount = params[LEVEL1_PARAM + i].getValue();
			if (inputs[CV1_INPUT + i].isConnected())
				amount *= clamp(inputs[CV1_INPUT + i].getVoltage() / 10.f, 0.f, 1.f);

			const float out = in * VcaCore::gainCurve(amount, expo);
			outputs[OUT1_OUTPUT + i].setVoltage(out);

			// A channel with its own cable patched is taken as routed away,
			// so it leaves the mix. Without this a patched channel would be
			// heard twice.
			if (!outputs[OUT1_OUTPUT + i].isConnected())
				mix += out;
		}

		// Saturate in normalised space so PRESSURE behaves the same whatever
		// the voltage range, then scale back to Rack's +/-10 V.
		const float sat = VcaCore::saturate(mix / 10.f, pressure) * 10.f;
		outputs[MIX_OUTPUT].setVoltage(sat);
	}
};

struct QuadVCAWidget : ModuleWidget {
	QuadVCAWidget(QuadVCA* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/QuadVCA.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// 12 HP = 60.96 mm. Four channel rows, one column header, then the
		// master section. Clearances were computed before the panel was drawn
		// rather than assumed - see the comment block in res/QuadVCA.svg for
		// the full arithmetic. Tightest pair: knob to CV jack, 4.24 mm.
		const float xLevel = 9.f, xCv = 23.33f, xIn = 37.67f, xOut = 52.f;
		const float rowY[4] = {30.f, 47.f, 64.f, 81.f};

		for (int i = 0; i < 4; i++) {
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xLevel, rowY[i])), module, QuadVCA::LEVEL1_PARAM + i));
			addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(xCv,    rowY[i])), module, QuadVCA::CV1_INPUT + i));
			addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(xIn,    rowY[i])), module, QuadVCA::IN1_INPUT + i));
			addOutput(createOutputCentered<PJ301MPort>(       mm2px(Vec(xOut,   rowY[i])), module, QuadVCA::OUT1_OUTPUT + i));
		}

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 101.60f)), module, QuadVCA::PRESSURE_PARAM));
		addParam(createParamCentered<CKSS>(               mm2px(Vec(45.72f, 101.60f)), module, QuadVCA::CURVE_PARAM));
		addOutput(createOutputCentered<PJ301MPort>(       mm2px(Vec(30.48f, 118.53f)), module, QuadVCA::MIX_OUTPUT));
	}
};

Model* modelQuadVCA = createModel<QuadVCA, QuadVCAWidget>("QuadVCA");
