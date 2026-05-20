#include "plugin.hpp"

// ============================================================
// DRIFT — slow random modulation source
//
// Design: Amplified Futures (steel finish, 12 HP)
//   PARAMS: RATE · WANDER · SLEW  (each with attenuverter + CV)
//   INPUTS: RATE CV · WANDER CV · SLEW CV · SYNC · V/OCT
//   OUTPUTS: SMOOTH · STEP · GATE · V/OCT (thru)
//
// DSP:
//   Phase accumulator advances at RATE Hz (0.01–10 Hz, exp)
//   On each step: target += WANDER × random [-1,1], clamped ±1
//   SMOOTH: 1-pole LP toward target (SLEW: 1000→0.1 Hz)
//   STEP:   current target (unsmoothed)
//   GATE:   10 V pulse, 5 ms exponential decay, on each step
// ============================================================

struct Drift : Module {
	enum ParamId {
		RATE_PARAM,
		WANDER_PARAM,
		SLEW_PARAM,
		RATE_ATTEN_PARAM,
		WANDER_ATTEN_PARAM,
		SLEW_ATTEN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RATE_CV_INPUT,
		WANDER_CV_INPUT,
		SLEW_CV_INPUT,
		SYNC_INPUT,
		VOCT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SMOOTH_OUTPUT,
		STEP_OUTPUT,
		GATE_OUTPUT,
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float phase   = 0.f;
	float target  = 0.f;
	float current = 0.f;
	float gate    = 0.f;
	dsp::SchmittTrigger syncTrig;

	Drift() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM,          0.f, 1.f, 0.35f, "Rate",   "%", 0.f, 100.f);
		configParam(WANDER_PARAM,        0.f, 1.f, 0.5f,  "Wander", "%", 0.f, 100.f);
		configParam(SLEW_PARAM,          0.f, 1.f, 0.5f,  "Slew",   "%", 0.f, 100.f);
		configParam(RATE_ATTEN_PARAM,   -1.f, 1.f, 0.f,   "Rate attenuverter");
		configParam(WANDER_ATTEN_PARAM, -1.f, 1.f, 0.f,   "Wander attenuverter");
		configParam(SLEW_ATTEN_PARAM,   -1.f, 1.f, 0.f,   "Slew attenuverter");
		getParamQuantity(RATE_PARAM)->description   = "Step rate: 0 = 0.01 Hz, 1 = 10 Hz (exponential)";
		getParamQuantity(WANDER_PARAM)->description = "Random walk step size per step. 0 = frozen, 1 = wild";
		getParamQuantity(SLEW_PARAM)->description   = "Smoothing lag. 0 = stepped (instant), 1 = very smooth";
		configInput(RATE_CV_INPUT,   "Rate CV");
		configInput(WANDER_CV_INPUT, "Wander CV");
		configInput(SLEW_CV_INPUT,   "Slew CV");
		configInput(SYNC_INPUT,      "Sync / reset trigger");
		configInput(VOCT_INPUT,      "V/oct (thru)");
		configOutput(SMOOTH_OUTPUT,  "Smooth random (±5 V)");
		configOutput(STEP_OUTPUT,    "Stepped random (±5 V)");
		configOutput(GATE_OUTPUT,    "Step gate (10 V, 5 ms)");
		configOutput(VOCT_OUTPUT,    "V/oct (thru)");
	}

	// Clamp a base param ± attenuated CV to [lo, hi]
	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		float rateP   = modp(RATE_PARAM,   RATE_ATTEN_PARAM,   RATE_CV_INPUT,   0.f, 1.f);
		float wanderP = modp(WANDER_PARAM, WANDER_ATTEN_PARAM, WANDER_CV_INPUT, 0.f, 1.f);
		float slewP   = modp(SLEW_PARAM,   SLEW_ATTEN_PARAM,   SLEW_CV_INPUT,   0.f, 1.f);

		// SYNC: rising edge forces immediate step
		if (syncTrig.process(inputs[SYNC_INPUT].getVoltage()))
			phase = 1.f;

		// Rate: 0.01 Hz to 10 Hz (exponential)
		float rateHz = 0.01f * std::pow(1000.f, rateP);
		phase += rateHz * args.sampleTime;

		if (phase >= 1.f) {
			phase -= 1.f;
			target += wanderP * (2.f * random::uniform() - 1.f);
			target = clamp(target, -1.f, 1.f);
			gate = 10.f;
		}

		// 5 ms gate decay
		gate *= std::exp(-args.sampleTime / 0.005f);

		// Slew: 1000 Hz (instant) → 0.1 Hz (very smooth)
		float slewHz   = 1000.f * std::pow(0.0001f, slewP);
		float slewAlpha = 1.f - std::exp(-2.f * float(M_PI) * slewHz * args.sampleTime);
		current += slewAlpha * (target - current);

		outputs[SMOOTH_OUTPUT].setVoltage(current * 5.f);
		outputs[STEP_OUTPUT].setVoltage(target * 5.f);
		outputs[GATE_OUTPUT].setVoltage(gate);
		outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage());
	}
};

// ============================================================
// WIDGET
// ============================================================

struct DriftWidget : ModuleWidget {
	DriftWidget(Drift* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Drift.svg")));

		// 12HP screws (4 corners)
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Left col params: RATE and WANDER
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 38.f)), module, Drift::RATE_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec( 9.24f, 50.f)), module, Drift::RATE_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(21.24f, 50.f)), module, Drift::RATE_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 68.f)), module, Drift::WANDER_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec( 9.24f, 80.f)), module, Drift::WANDER_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(21.24f, 80.f)), module, Drift::WANDER_CV_INPUT));

		// Right col: SLEW + utility I/O
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(45.72f, 38.f)), module, Drift::SLEW_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(39.72f, 50.f)), module, Drift::SLEW_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(51.72f, 50.f)), module, Drift::SLEW_CV_INPUT));

		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(45.72f, 68.f)), module, Drift::SYNC_INPUT));
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(45.72f, 85.f)), module, Drift::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.72f, 100.f)), module, Drift::VOCT_OUTPUT));

		// Bottom outputs: SMOOTH, STEP, GATE
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24f, 114.f)), module, Drift::SMOOTH_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.48f, 114.f)), module, Drift::STEP_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.72f, 114.f)), module, Drift::GATE_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 13.f);
		nvgTextLetterSpacing(args.vg, 5.f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "DRIFT", NULL);
	}
};

Model* modelDrift = createModel<Drift, DriftWidget>("Drift");
