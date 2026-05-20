#include "plugin.hpp"

// ============================================================
// WALL CONDUCTOR — section-based performance mixer/conductor
//
// Design: Amplified Futures (steel finish, 22 HP)
//   DENSITY: sweeps channels 1-4 in/out (one at a time)
//   PRESSURE: drive into tanh saturation
//   WIDTH: stereo spread via constant-power channel panning
//   FEEDBACK: 1-sample output→input feedback bus
//   RECOVERY: rise time after COLLAPSE (50ms–10s)
//   COLLAPSE: button or gate, instant duck with shaped recovery
//
// Signal flow:
//   mix += feedback * FEEDBACK
//   for each CH: gain = clamp(DENSITY*4 - i, 0, 1)
//                pan  = pans[i] * WIDTH
//                mixL += sig * cos(panR * π/2)
//                mixR += sig * sin(panR * π/2)
//   out = 5 * tanh(mix * drive / 5) * collapseEnv
// ============================================================

struct WallConductor : Module {
	enum ParamId {
		DENSITY_PARAM,
		PRESSURE_PARAM,
		WIDTH_PARAM,
		FEEDBACK_PARAM,
		RECOVERY_PARAM,
		COLLAPSE_PARAM,
		DENSITY_ATTEN_PARAM,
		PRESSURE_ATTEN_PARAM,
		WIDTH_ATTEN_PARAM,
		FEEDBACK_ATTEN_PARAM,
		RECOVERY_ATTEN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CH1_INPUT,
		CH2_INPUT,
		CH3_INPUT,
		CH4_INPUT,
		DENSITY_CV_INPUT,
		PRESSURE_CV_INPUT,
		WIDTH_CV_INPUT,
		FEEDBACK_CV_INPUT,
		RECOVERY_CV_INPUT,
		COLLAPSE_INPUT,
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

	float collapseEnv = 1.f;
	float feedbackL   = 0.f;
	float feedbackR   = 0.f;

	WallConductor() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(DENSITY_PARAM,  0.f, 1.f, 1.0f, "Density",  "%", 0.f, 100.f);
		configParam(PRESSURE_PARAM, 0.f, 1.f, 0.3f, "Pressure", "%", 0.f, 100.f);
		configParam(WIDTH_PARAM,    0.f, 1.f, 0.7f, "Width",    "%", 0.f, 100.f);
		configParam(FEEDBACK_PARAM, 0.f, 1.f, 0.0f, "Feedback", "%", 0.f, 100.f);
		configParam(RECOVERY_PARAM, 0.f, 1.f, 0.3f, "Recovery", "%", 0.f, 100.f);
		configButton(COLLAPSE_PARAM, "Collapse");
		configParam(DENSITY_ATTEN_PARAM,  -1.f, 1.f, 0.f, "Density attenuverter");
		configParam(PRESSURE_ATTEN_PARAM, -1.f, 1.f, 0.f, "Pressure attenuverter");
		configParam(WIDTH_ATTEN_PARAM,    -1.f, 1.f, 0.f, "Width attenuverter");
		configParam(FEEDBACK_ATTEN_PARAM, -1.f, 1.f, 0.f, "Feedback attenuverter");
		configParam(RECOVERY_ATTEN_PARAM, -1.f, 1.f, 0.f, "Recovery attenuverter");
		configInput(CH1_INPUT,        "Channel 1");
		configInput(CH2_INPUT,        "Channel 2");
		configInput(CH3_INPUT,        "Channel 3");
		configInput(CH4_INPUT,        "Channel 4");
		configInput(DENSITY_CV_INPUT, "Density CV");
		configInput(PRESSURE_CV_INPUT,"Pressure CV");
		configInput(WIDTH_CV_INPUT,   "Width CV");
		configInput(FEEDBACK_CV_INPUT,"Feedback CV");
		configInput(RECOVERY_CV_INPUT,"Recovery CV");
		configInput(COLLAPSE_INPUT,   "Collapse gate");
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

	float sumInput(int id) {
		int ch = inputs[id].getChannels();
		float s = 0.f;
		for (int c = 0; c < ch; c++) s += inputs[id].getVoltage(c);
		return s;
	}

	void process(const ProcessArgs& args) override {
		float density  = modp(DENSITY_PARAM,  DENSITY_ATTEN_PARAM,  DENSITY_CV_INPUT,  0.f, 1.f);
		float pressure = modp(PRESSURE_PARAM, PRESSURE_ATTEN_PARAM, PRESSURE_CV_INPUT, 0.f, 1.f);
		float width    = modp(WIDTH_PARAM,    WIDTH_ATTEN_PARAM,    WIDTH_CV_INPUT,    0.f, 1.f);
		float feedback = modp(FEEDBACK_PARAM, FEEDBACK_ATTEN_PARAM, FEEDBACK_CV_INPUT, 0.f, 0.92f);
		float recovery = modp(RECOVERY_PARAM, RECOVERY_ATTEN_PARAM, RECOVERY_CV_INPUT, 0.f, 1.f);

		bool collapseActive = (params[COLLAPSE_PARAM].getValue() > 0.5f)
		                   || (inputs[COLLAPSE_INPUT].getVoltage() >= 1.f);
		float collapseTarget = collapseActive ? 0.f : 1.f;
		float attackTime   = 0.002f;
		float recoveryTime = 0.05f + recovery * 9.95f;
		float tau = (collapseTarget < collapseEnv) ? attackTime : recoveryTime;
		collapseEnv += (collapseTarget - collapseEnv) * (1.f - std::exp(-args.sampleTime / tau));

		// Fixed panning positions for the 4 channels (L–center-L–center-R–R)
		const float pans[4]  = { -1.f, -0.33f, 0.33f, 1.f };
		const int   chIds[4] = { CH1_INPUT, CH2_INPUT, CH3_INPUT, CH4_INPUT };

		float mixL = feedbackL * feedback;
		float mixR = feedbackR * feedback;

		for (int i = 0; i < 4; i++) {
			float gain = clamp(density * 4.f - float(i), 0.f, 1.f);
			float sig  = sumInput(chIds[i]) * gain;
			float panR = (pans[i] * width + 1.f) * 0.5f;
			mixL += sig * std::cos(panR * float(M_PI) * 0.5f);
			mixR += sig * std::sin(panR * float(M_PI) * 0.5f);
		}

		float drive = 1.f + pressure * 3.f;
		float outL = 5.f * std::tanh(mixL * drive / 5.f) * collapseEnv;
		float outR = 5.f * std::tanh(mixR * drive / 5.f) * collapseEnv;

		feedbackL = outL;
		feedbackR = outR;

		outputs[OUT_L_OUTPUT].setVoltage(outL);
		outputs[OUT_R_OUTPUT].setVoltage(outR);
		outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage());
	}
};

// ============================================================
// WIDGET  (22 HP)
// ============================================================

struct WallConductorWidget : ModuleWidget {
	WallConductorWidget(WallConductor* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/WallConductor.svg")));

		// 22HP screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// ── Row 1: DENSITY | PRESSURE | WIDTH ─────────────────────
		// x cols for knobs: 20, 56, 92 mm
		// x cols for satellites: 28, 64, 100 mm (knob + 8mm)

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(20.f, 45.f)), module, WallConductor::DENSITY_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(28.f, 38.f)), module, WallConductor::DENSITY_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(28.f, 52.f)), module, WallConductor::DENSITY_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(56.f, 45.f)), module, WallConductor::PRESSURE_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(64.f, 38.f)), module, WallConductor::PRESSURE_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(64.f, 52.f)), module, WallConductor::PRESSURE_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(92.f, 45.f)), module, WallConductor::WIDTH_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(100.f, 38.f)), module, WallConductor::WIDTH_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(100.f, 52.f)), module, WallConductor::WIDTH_CV_INPUT));

		// ── Row 2: FEEDBACK | RECOVERY | COLLAPSE ─────────────────

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(20.f, 72.f)), module, WallConductor::FEEDBACK_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(28.f, 65.f)), module, WallConductor::FEEDBACK_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(28.f, 79.f)), module, WallConductor::FEEDBACK_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(56.f, 72.f)), module, WallConductor::RECOVERY_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(64.f, 65.f)), module, WallConductor::RECOVERY_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(64.f, 79.f)), module, WallConductor::RECOVERY_CV_INPUT));

		// COLLAPSE: button stacked above gate input
		addParam(createParamCentered<TL1105>(    mm2px(Vec(92.f, 66.f)), module, WallConductor::COLLAPSE_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(92.f, 79.f)), module, WallConductor::COLLAPSE_INPUT));

		// ── Row 3: Channel inputs ──────────────────────────────────
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.f,  108.f)), module, WallConductor::CH1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.f,  108.f)), module, WallConductor::CH2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(61.f,  108.f)), module, WallConductor::CH3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(84.f,  108.f)), module, WallConductor::CH4_INPUT));

		// ── Row 4: V/OCT thru + stereo out ────────────────────────
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(15.f,  120.f)), module, WallConductor::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.f,  120.f)), module, WallConductor::VOCT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(75.f,  120.f)), module, WallConductor::OUT_L_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(97.f,  120.f)), module, WallConductor::OUT_R_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 11.f);
		nvgTextLetterSpacing(args.vg, 2.5f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "WALL CONDUCTOR", NULL);
	}
};

Model* modelWallConductor = createModel<WallConductor, WallConductorWidget>("WallConductor");
