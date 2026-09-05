// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include "dsp/EnvCore.hpp"

// ============================================================
// COLLAPSE EG (AF-04) — 8 HP attack/decay envelope
//
// The series had no standalone envelope: Pulse has internal envelopes but
// nothing could shape an arbitrary signal. This is that module.
//
//   PARAMS:  ATTACK · DECAY · CURVE · MISFIRE · LOOP (switch)
//   INPUTS:  GATE · TRIG
//   OUTPUTS: ENV · INV · EOC
//
// MISFIRE at 0 is a textbook AD envelope, exactly. The character is
// opt-in, as with BROWNOUT on the clock. EnvCore is unit-tested offline
// in tests/test_env_core.cpp.
// ============================================================

struct AFOrangeLightEG : GrayModuleLightWidget {
	AFOrangeLightEG() { addBaseColor(nvgRGB(0xFF, 0x4A, 0x0E)); }
};

struct CollapseEG : Module {
	enum ParamId {
		ATTACK_PARAM,
		DECAY_PARAM,
		CURVE_PARAM,
		MISFIRE_PARAM,
		LOOP_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		GATE_INPUT,
		TRIG_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENV_OUTPUT,
		INV_OUTPUT,
		EOC_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENV_LIGHT,
		LIGHTS_LEN
	};

	EnvCore core;
	dsp::SchmittTrigger gateTrig, trigTrig;
	dsp::PulseGenerator eocPulse;

	CollapseEG() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.1f, "Attack");
		configParam(DECAY_PARAM, 0.f, 1.f, 0.4f, "Decay");
		configParam(CURVE_PARAM, -1.f, 1.f, 0.f, "Curve", "", 0.f, 1.f);
		configParam(MISFIRE_PARAM, 0.f, 1.f, 0.f, "Misfire", "%", 0.f, 100.f);
		configSwitch(LOOP_PARAM, 0.f, 1.f, 0.f, "Loop", {"Off", "On"});
		configInput(GATE_INPUT, "Gate");
		configInput(TRIG_INPUT, "Trigger");
		configOutput(ENV_OUTPUT, "Envelope");
		configOutput(INV_OUTPUT, "Inverted envelope");
		configOutput(EOC_OUTPUT, "End of cycle");
		core.setSampleRate(44100.f);
		core.reset();
	}

	void onSampleRateChange() override {
		core.setSampleRate(APP->engine->getSampleRate());
	}

	void process(const ProcessArgs& args) override {
		const float misfire = params[MISFIRE_PARAM].getValue();
		const bool loop = params[LOOP_PARAM].getValue() > 0.5f;

		// ATTACK and DECAY are exponential so short percussive times sit under
		// the hand instead of crushed against the bottom of the sweep.
		const float attack = 0.0005f * std::pow(8000.f, params[ATTACK_PARAM].getValue());
		const float decay  = 0.0005f * std::pow(16000.f, params[DECAY_PARAM].getValue());
		const float curve  = params[CURVE_PARAM].getValue();

		if (trigTrig.process(inputs[TRIG_INPUT].getVoltage(), 0.1f, 2.f))
			core.trigger(misfire);
		if (gateTrig.process(inputs[GATE_INPUT].getVoltage(), 0.1f, 2.f))
			core.trigger(misfire);

		// LOOP re-fires as soon as the envelope finishes, turning the module
		// into an LFO. A misfired repeat simply skips a cycle, which is what
		// makes a looping Collapse EG stutter rather than run evenly.
		if (loop && !core.isRunning())
			core.trigger(misfire);

		bool eoc = false;
		const float env = core.process(attack, decay, curve, &eoc);
		if (eoc) eocPulse.trigger(1e-3f);

		outputs[ENV_OUTPUT].setVoltage(env * 10.f);
		outputs[INV_OUTPUT].setVoltage((1.f - env) * 10.f);
		outputs[EOC_OUTPUT].setVoltage(eocPulse.process(args.sampleTime) ? 10.f : 0.f);
		lights[ENV_LIGHT].setBrightness(env);
	}
};

struct CollapseEGWidget : ModuleWidget {
	CollapseEGWidget(CollapseEG* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/CollapseEG.svg")));


		// 8 HP = 40.64 mm. One centred column of knobs, I/O in two rows at the
		// foot. Labels go ABOVE every widget.
		//
		// Clearances checked arithmetically before the panel was drawn, not
		// discovered in the render: MISFIRE bottom 90.74mm vs the GATE label at
		// 94.65mm; input bottom 107.35mm vs the output label at 109.65mm; output
		// bottom 122.35mm vs screws beginning at 123.60mm.
		const float xc = 20.32f;

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xc, 26.f)), module, CollapseEG::ATTACK_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xc, 46.f)), module, CollapseEG::DECAY_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xc, 66.f)), module, CollapseEG::CURVE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(xc, 86.f)), module, CollapseEG::MISFIRE_PARAM));

		addChild(createLightCentered<SmallLight<AFOrangeLightEG>>(mm2px(Vec(32.f, 26.f)), module, CollapseEG::ENV_LIGHT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(32.f, 66.f)), module, CollapseEG::LOOP_PARAM));

		addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(10.16f, 102.f)), module, CollapseEG::GATE_INPUT));
		addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(30.48f, 102.f)), module, CollapseEG::TRIG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec( 7.5f, 117.f)), module, CollapseEG::ENV_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32f, 117.f)), module, CollapseEG::INV_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.14f, 117.f)), module, CollapseEG::EOC_OUTPUT));
	}
};

Model* modelCollapseEG = createModel<CollapseEG, CollapseEGWidget>("CollapseEG");
