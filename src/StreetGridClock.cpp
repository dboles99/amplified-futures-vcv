// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include "dsp/ClockCore.hpp"

// ============================================================
// STREET GRID CLOCK (AF-02) — 12 HP master clock
//
// The Branca series had no clock at all: a patch built only from these
// modules could not generate its own time. This is that module.
//
//   PARAMS:  RATE · SWING · BROWNOUT  (each with attenuverter + CV)
//            RUN (toggle) · RESET (momentary)
//   INPUTS:  RATE CV · SWING CV · BROWNOUT CV · EXT CLK · RESET
//   OUTPUTS: CLK · /2 · /4 · /8 · RESET
//
// BROWNOUT is a mains-sag metaphor: the grid dips under load and recovers.
// At 0 it is EXACTLY steady — see ClockCore, which is unit-tested offline
// in tests/test_clock_core.cpp.
// ============================================================

struct StreetGridClock : Module {
	enum ParamId {
		RATE_PARAM,
		SWING_PARAM,
		BROWNOUT_PARAM,
		RATE_ATTEN_PARAM,
		SWING_ATTEN_PARAM,
		BROWNOUT_ATTEN_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RATE_CV_INPUT,
		SWING_CV_INPUT,
		BROWNOUT_CV_INPUT,
		EXT_CLK_INPUT,
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		CLK_OUTPUT,
		DIV2_OUTPUT,
		DIV4_OUTPUT,
		DIV8_OUTPUT,
		RESET_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		RUN_LIGHT,
		LIGHTS_LEN
	};

	ClockCore core;
	bool running = true;
	dsp::BooleanTrigger runTrig, resetBtnTrig;
	dsp::SchmittTrigger extTrig, resetInTrig;
	dsp::PulseGenerator pulses[5];

	StreetGridClock() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 0.4f, "Rate");
		configParam(SWING_PARAM, 0.f, 0.75f, 0.f, "Swing", "%", 0.f, 100.f);
		configParam(BROWNOUT_PARAM, 0.f, 1.f, 0.f, "Brownout", "%", 0.f, 100.f);
		configParam(RATE_ATTEN_PARAM, -1.f, 1.f, 0.f, "Rate CV attenuverter");
		configParam(SWING_ATTEN_PARAM, -1.f, 1.f, 0.f, "Swing CV attenuverter");
		configParam(BROWNOUT_ATTEN_PARAM, -1.f, 1.f, 0.f, "Brownout CV attenuverter");
		configButton(RUN_PARAM, "Run");
		configButton(RESET_PARAM, "Reset");
		configInput(RATE_CV_INPUT, "Rate CV");
		configInput(SWING_CV_INPUT, "Swing CV");
		configInput(BROWNOUT_CV_INPUT, "Brownout CV");
		configInput(EXT_CLK_INPUT, "External clock");
		configInput(RESET_INPUT, "Reset");
		configOutput(CLK_OUTPUT, "Clock");
		configOutput(DIV2_OUTPUT, "Clock /2");
		configOutput(DIV4_OUTPUT, "Clock /4");
		configOutput(DIV8_OUTPUT, "Clock /8");
		configOutput(RESET_OUTPUT, "Reset");
		core.setSampleRate(44100.f);
		core.reset();
	}

	// Workspace-standard helper: param + attenuverter × CV, normalised 0..1,
	// then scaled to the parameter's real range.
	float modp(int p, int a, int c, float lo, float hi) {
		float base = params[p].getValue();
		float atten = params[a].getValue();
		float cv = inputs[c].getVoltage() / 10.f;
		return clamp(base + atten * cv, 0.f, 1.f) * (hi - lo) + lo;
	}

	void onSampleRateChange() override {
		core.setSampleRate(APP->engine->getSampleRate());
	}

	// Run/stop is latched by the RUN trigger rather than held in a parameter.
	json_t* dataToJson() override {
		json_t* root = json_object();
		json_object_set_new(root, "running", json_boolean(running));
		return root;
	}

	void dataFromJson(json_t* root) override {
		json_t* v = json_object_get(root, "running");
		if (v)
			running = json_boolean_value(v);
	}

	void process(const ProcessArgs& args) override {
		if (runTrig.process(params[RUN_PARAM].getValue() > 0.5f))
			running = !running;
		lights[RUN_LIGHT].setBrightness(running ? 1.f : 0.f);

		bool doReset = false;
		if (resetBtnTrig.process(params[RESET_PARAM].getValue() > 0.5f))
			doReset = true;
		if (resetInTrig.process(inputs[RESET_INPUT].getVoltage(), 0.1f, 2.f))
			doReset = true;
		if (doReset) {
			core.reset();
			pulses[4].trigger(1e-3f);
		}

		// RATE is exponential across 20–300 BPM so the useful range sits under
		// the hand rather than bunched at one end of the sweep.
		const float rate01 = modp(RATE_PARAM, RATE_ATTEN_PARAM, RATE_CV_INPUT, 0.f, 1.f);
		const float bpm = 20.f * std::pow(15.f, rate01);   // 20 → 300
		const float swing = modp(SWING_PARAM, SWING_ATTEN_PARAM, SWING_CV_INPUT, 0.f, 0.75f);
		const float brown = modp(BROWNOUT_PARAM, BROWNOUT_ATTEN_PARAM, BROWNOUT_CV_INPUT, 0.f, 1.f);

		ClockCore::Ticks t;
		if (inputs[EXT_CLK_INPUT].isConnected()) {
			// An external clock drives the divisions directly; RATE is ignored
			// so the divisions stay locked to the incoming edges.
			if (extTrig.process(inputs[EXT_CLK_INPUT].getVoltage(), 0.1f, 2.f) && running)
				t = core.tickExternal();
		}
		else {
			t = core.process(bpm, swing, brown, running);
		}

		if (t.clk)  pulses[0].trigger(1e-3f);
		if (t.div2) pulses[1].trigger(1e-3f);
		if (t.div4) pulses[2].trigger(1e-3f);
		if (t.div8) pulses[3].trigger(1e-3f);

		outputs[CLK_OUTPUT].setVoltage(pulses[0].process(args.sampleTime) ? 10.f : 0.f);
		outputs[DIV2_OUTPUT].setVoltage(pulses[1].process(args.sampleTime) ? 10.f : 0.f);
		outputs[DIV4_OUTPUT].setVoltage(pulses[2].process(args.sampleTime) ? 10.f : 0.f);
		outputs[DIV8_OUTPUT].setVoltage(pulses[3].process(args.sampleTime) ? 10.f : 0.f);
		outputs[RESET_OUTPUT].setVoltage(pulses[4].process(args.sampleTime) ? 10.f : 0.f);
	}
};

struct StreetGridClockWidget : ModuleWidget {
	StreetGridClockWidget(StreetGridClock* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/StreetGridClock.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Satellite layout copied from Drift.cpp, which scored 33/35 — the
		// highest in the set: knob at (x, y), attenuverter at (x-6, y+12),
		// CV jack at (x+6, y+12). Labels go ABOVE the knob, never at its
		// centre; that mistake is what broke ten panels.
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 30.f)), module, StreetGridClock::RATE_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec( 9.24f, 42.f)), module, StreetGridClock::RATE_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(21.24f, 42.f)), module, StreetGridClock::RATE_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(45.72f, 30.f)), module, StreetGridClock::SWING_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(39.72f, 42.f)), module, StreetGridClock::SWING_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(51.72f, 42.f)), module, StreetGridClock::SWING_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 60.f)), module, StreetGridClock::BROWNOUT_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec( 9.24f, 72.f)), module, StreetGridClock::BROWNOUT_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(21.24f, 72.f)), module, StreetGridClock::BROWNOUT_CV_INPUT));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(45.72f, 52.f)), module, StreetGridClock::RUN_LIGHT));
		addParam(createParamCentered<TL1105>(mm2px(Vec(45.72f, 60.f)), module, StreetGridClock::RUN_PARAM));
		addParam(createParamCentered<TL1105>(mm2px(Vec(45.72f, 72.f)), module, StreetGridClock::RESET_PARAM));

		addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(12.f, 92.f)), module, StreetGridClock::EXT_CLK_INPUT));
		addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(30.f, 92.f)), module, StreetGridClock::RESET_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48.f, 92.f)), module, StreetGridClock::RESET_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec( 9.0f, 112.f)), module, StreetGridClock::CLK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.5f, 112.f)), module, StreetGridClock::DIV2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36.0f, 112.f)), module, StreetGridClock::DIV4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(49.5f, 112.f)), module, StreetGridClock::DIV8_OUTPUT));
	}
};

Model* modelStreetGridClock =
	createModel<StreetGridClock, StreetGridClockWidget>("StreetGridClock");
