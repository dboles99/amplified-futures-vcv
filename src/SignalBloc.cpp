// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include "dsp/BlocCore.hpp"

// ============================================================
// SIGNAL BLOC (AF-06) — 10 HP CV glue
//
// Three unrelated utilities that a patch always ends up needing:
//
//   1. Two attenuverter + offset channels:  OUT = IN x ATT + OFF
//   2. A precision three-input adder
//   3. A buffered 1-to-3 mult
//
// No character knob, by design. This is the module you reach for when you
// want to know exactly what happened to a voltage.
//
// Everything is polyphonic. On a mult and an adder polyphony genuinely does
// fall out for free - copy the channel count, loop the arithmetic - so the
// series' "no polyphony unless it is free" rule permits it here.
//
// Width note: the spec sketched this at 8 HP. Sixteen widgets do not fit in
// 8 HP without putting every clearance at its minimum, which is the exact
// condition that produced the AF-04 collision. At 10 HP each channel becomes
// a single ATT/OFF/IN/OUT row, which removes two rows outright. See the
// arithmetic in res/SignalBloc.svg.
//
// BlocCore is unit-tested offline in tests/test_bloc_core.cpp.
// ============================================================

struct SignalBloc : Module {
	enum ParamId {
		ATT1_PARAM, OFF1_PARAM,
		ATT2_PARAM, OFF2_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		IN1_INPUT, IN2_INPUT,
		SUM_A_INPUT, SUM_B_INPUT, SUM_C_INPUT,
		MULT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT1_OUTPUT, OUT2_OUTPUT,
		SUM_OUTPUT,
		MULT1_OUTPUT, MULT2_OUTPUT, MULT3_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	SignalBloc() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int ch = 0; ch < 2; ch++) {
			// Unity and zero: the module is transparent until you ask it not
			// to be, which is the same default the rest of the series takes.
			configParam(ATT1_PARAM + ch * 2, -1.f, 1.f, 1.f,
				string::f("Channel %d attenuvert", ch + 1));
			configParam(OFF1_PARAM + ch * 2, -5.f, 5.f, 0.f,
				string::f("Channel %d offset", ch + 1), " V");
			configInput(IN1_INPUT + ch, string::f("Channel %d", ch + 1));
			configOutput(OUT1_OUTPUT + ch, string::f("Channel %d", ch + 1));
		}
		configInput(SUM_A_INPUT, "Sum A");
		configInput(SUM_B_INPUT, "Sum B");
		configInput(SUM_C_INPUT, "Sum C");
		configOutput(SUM_OUTPUT, "Sum");
		configInput(MULT_INPUT, "Mult");
		configOutput(MULT1_OUTPUT, "Mult 1");
		configOutput(MULT2_OUTPUT, "Mult 2");
		configOutput(MULT3_OUTPUT, "Mult 3");
	}

	void process(const ProcessArgs& args) override {
		// --- two attenuverter + offset channels ---------------------------
		for (int ch = 0; ch < 2; ch++) {
			const float att = params[ATT1_PARAM + ch * 2].getValue();
			const float off = params[OFF1_PARAM + ch * 2].getValue();
			Input& in = inputs[IN1_INPUT + ch];
			Output& out = outputs[OUT1_OUTPUT + ch];

			// An unpatched input leaves the channel as a constant-voltage
			// source, which is half of what an offset utility is for.
			const int n = std::max(1, in.getChannels());
			out.setChannels(n);
			for (int c = 0; c < n; c++)
				out.setVoltage(BlocCore::scaleOffset(in.getVoltage(c), att, off), c);
		}

		// --- precision three-input adder -----------------------------------
		{
			const int n = std::max(std::max(1, inputs[SUM_A_INPUT].getChannels()),
			                       std::max(inputs[SUM_B_INPUT].getChannels(),
			                                inputs[SUM_C_INPUT].getChannels()));
			outputs[SUM_OUTPUT].setChannels(n);
			for (int c = 0; c < n; c++) {
				// getPolyVoltage, not getVoltage: a mono cable into a poly sum
				// should feed every channel rather than only the first.
				outputs[SUM_OUTPUT].setVoltage(
					BlocCore::sum3(inputs[SUM_A_INPUT].getPolyVoltage(c),
					               inputs[SUM_B_INPUT].getPolyVoltage(c),
					               inputs[SUM_C_INPUT].getPolyVoltage(c)), c);
			}
		}

		// --- buffered 1-to-3 mult -------------------------------------------
		{
			Input& in = inputs[MULT_INPUT];
			const int n = std::max(1, in.getChannels());
			for (int o = 0; o < 3; o++) {
				Output& out = outputs[MULT1_OUTPUT + o];
				out.setChannels(n);
				for (int c = 0; c < n; c++)
					out.setVoltage(BlocCore::clampRail(in.getVoltage(c)), c);
			}
		}
	}
};

struct SignalBlocWidget : ModuleWidget {
	SignalBlocWidget(SignalBloc* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/SignalBloc.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// 10 HP = 50.8 mm. Clearances were computed before the panel was drawn
		// - the full arithmetic is in res/SignalBloc.svg. Tightest pair is the
		// two channel rows at 10.2 px; the widest row (ATT/OFF/IN/OUT) leaves
		// 6.0-6.4 px between widgets, matching the 3-port row already shipped
		// on AF-04.
		const float xAtt = 6.774f, xOff = 18.288f, xIn = 30.480f, xOut = 43.351f;
		const float rowY[2] = {25.739f, 39.963f};

		for (int ch = 0; ch < 2; ch++) {
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xAtt, rowY[ch])), module, SignalBloc::ATT1_PARAM + ch * 2));
			addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xOff, rowY[ch])), module, SignalBloc::OFF1_PARAM + ch * 2));
			addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(xIn,  rowY[ch])), module, SignalBloc::IN1_INPUT + ch));
			addOutput(createOutputCentered<PJ301MPort>(       mm2px(Vec(xOut, rowY[ch])), module, SignalBloc::OUT1_OUTPUT + ch));
		}

		// Three-across rows: 10.161 / 25.400 / 40.640 mm.
		addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(10.161f, 66.377f)), module, SignalBloc::SUM_A_INPUT));
		addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(25.400f, 66.377f)), module, SignalBloc::SUM_B_INPUT));
		addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(40.640f, 66.377f)), module, SignalBloc::SUM_C_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.240f, 90.762f)), module, SignalBloc::SUM_OUTPUT));
		addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(35.560f, 90.762f)), module, SignalBloc::MULT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.161f, 115.147f)), module, SignalBloc::MULT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(25.400f, 115.147f)), module, SignalBloc::MULT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(40.640f, 115.147f)), module, SignalBloc::MULT3_OUTPUT));
	}
};

Model* modelSignalBloc = createModel<SignalBloc, SignalBlocWidget>("SignalBloc");
