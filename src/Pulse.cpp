#include "plugin.hpp"

// ============================================================
// PULSE — 16-step no-wave step percussion
//
// Design: Amplified Futures (steel finish, 12 HP)
//   GRID:   4×4 toggle step buttons with GreenRed LED feedback
//   PARAMS: HIT · DECAY · METAL · CRACK (each with CV + atten)
//   IO:     TRG clock · V/OCT thru · audio OUT
//
// Synthesis per triggered step:
//   White noise → 1-pole LP (body, METAL shifts 360→80 Hz)
//   + raw noise × METAL + noise × crack transient (4 ms)
//   × exponential envelope (DECAY 8–500 ms) × HIT → 2× tanh
// ============================================================

struct Pulse : Module {
	enum ParamId {
		// existing step buttons (IDs preserved)
		ENUMS(STEP_PARAM, 16),
		HIT_PARAM,
		DECAY_PARAM,
		METAL_PARAM,
		CRACK_PARAM,
		// appended: attenuverters
		HIT_ATTEN_PARAM,
		DECAY_ATTEN_PARAM,
		METAL_ATTEN_PARAM,
		CRACK_ATTEN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		// existing
		TRG_INPUT,
		// appended
		HIT_CV_INPUT,
		DECAY_CV_INPUT,
		METAL_CV_INPUT,
		CRACK_CV_INPUT,
		VOCT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		// existing
		OUT_OUTPUT,
		// appended
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(STEP_LIGHT, 32),   // 16 × GreenRedLight
		LIGHTS_LEN
	};

	bool steps[16] = {};
	dsp::BooleanTrigger stepTrig[16];
	int currentStep = -1;
	dsp::SchmittTrigger trgTrig;

	float env      = 0.f;
	float crackEnv = 0.f;
	float noiseLP  = 0.f;

	Pulse() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int i = 0; i < 16; i++)
			configButton(STEP_PARAM + i, string::f("Step %d", i + 1));
		configParam(HIT_PARAM,          0.f, 1.f, 0.75f, "Hit level", "%", 0.f, 100.f);
		configParam(DECAY_PARAM,        0.f, 1.f, 0.30f, "Decay",     "%", 0.f, 100.f);
		configParam(METAL_PARAM,        0.f, 1.f, 0.20f, "Metal",     "%", 0.f, 100.f);
		configParam(CRACK_PARAM,        0.f, 1.f, 0.40f, "Crack",     "%", 0.f, 100.f);
		configParam(HIT_ATTEN_PARAM,   -1.f, 1.f, 0.f,  "Hit attenuverter");
		configParam(DECAY_ATTEN_PARAM, -1.f, 1.f, 0.f,  "Decay attenuverter");
		configParam(METAL_ATTEN_PARAM, -1.f, 1.f, 0.f,  "Metal attenuverter");
		configParam(CRACK_ATTEN_PARAM, -1.f, 1.f, 0.f,  "Crack attenuverter");
		configInput(TRG_INPUT,      "Trigger clock");
		configInput(HIT_CV_INPUT,   "Hit CV");
		configInput(DECAY_CV_INPUT, "Decay CV");
		configInput(METAL_CV_INPUT, "Metal CV");
		configInput(CRACK_CV_INPUT, "Crack CV");
		configInput(VOCT_INPUT,     "V/oct (thru)");
		configOutput(OUT_OUTPUT,    "Audio");
		configOutput(VOCT_OUTPUT,   "V/oct (thru)");
	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		for (int i = 0; i < 16; i++) {
			if (stepTrig[i].process(params[STEP_PARAM + i].getValue() > 0.5f))
				steps[i] = !steps[i];
			lights[STEP_LIGHT + i * 2 + 0].setBrightness(steps[i] ? 1.f : 0.f);
			lights[STEP_LIGHT + i * 2 + 1].setBrightness(i == currentStep ? 0.5f : 0.f);
		}

		if (trgTrig.process(inputs[TRG_INPUT].getVoltage())) {
			currentStep = (currentStep + 1) % 16;
			if (steps[currentStep]) {
				env      = 1.f;
				crackEnv = 1.f;
			}
		}

		float hit   = modp(HIT_PARAM,   HIT_ATTEN_PARAM,   HIT_CV_INPUT,   0.f, 1.f);
		float decay = modp(DECAY_PARAM, DECAY_ATTEN_PARAM, DECAY_CV_INPUT, 0.f, 1.f);
		float metal = modp(METAL_PARAM, METAL_ATTEN_PARAM, METAL_CV_INPUT, 0.f, 1.f);
		float crack = modp(CRACK_PARAM, CRACK_ATTEN_PARAM, CRACK_CV_INPUT, 0.f, 1.f);

		float noise = 2.f * random::uniform() - 1.f;

		float decayTime = 0.008f + decay * 0.492f;
		env      *= std::exp(-args.sampleTime / decayTime);
		crackEnv *= std::exp(-args.sampleTime / 0.004f);

		float bodyFreq  = 80.f + (1.f - metal) * 280.f;
		float bodyAlpha = 1.f - std::exp(-2.f * float(M_PI) * bodyFreq * args.sampleTime);
		noiseLP += bodyAlpha * (noise - noiseLP);

		float sig = noiseLP * (1.f - metal * 0.5f)
		          + noise   * metal
		          + noise   * crackEnv * crack;
		sig *= env * hit;

		outputs[OUT_OUTPUT].setVoltage(5.f * std::tanh(sig * 2.f));
		outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage());
	}
};

// ============================================================
// WIDGET  (12 HP)
// ============================================================

struct PulseWidget : ModuleWidget {
	PulseWidget(Pulse* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Pulse.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// 4×4 step grid — spread across 12HP
		const float stepX[4] = { 12.f, 22.f, 32.f, 42.f };
		const float stepY[4] = { 24.f, 36.f, 48.f, 60.f };

		for (int row = 0; row < 4; row++) {
			for (int col = 0; col < 4; col++) {
				int idx = row * 4 + col;
				addParam(createParamCentered<TL1105>(
					mm2px(Vec(stepX[col], stepY[row])), module, Pulse::STEP_PARAM + idx));
				addChild(createLightCentered<SmallLight<GreenRedLight>>(
					mm2px(Vec(stepX[col], stepY[row])), module, Pulse::STEP_LIGHT + idx * 2));
			}
		}

		// Params + satellites (right of each knob)
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.f, 76.f)), module, Pulse::HIT_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(19.f, 69.f)), module, Pulse::HIT_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(19.f, 83.f)), module, Pulse::HIT_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(42.f, 76.f)), module, Pulse::DECAY_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(49.f, 69.f)), module, Pulse::DECAY_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(49.f, 83.f)), module, Pulse::DECAY_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.f, 96.f)), module, Pulse::METAL_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(19.f, 89.f)), module, Pulse::METAL_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(19.f, 103.f)), module, Pulse::METAL_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(42.f, 96.f)), module, Pulse::CRACK_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(49.f, 89.f)), module, Pulse::CRACK_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(49.f, 103.f)), module, Pulse::CRACK_CV_INPUT));

		// IO row: TRG · V/OCT IN · V/OCT OUT · OUT
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(12.f, 115.f)), module, Pulse::TRG_INPUT));
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(24.f, 115.f)), module, Pulse::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(36.f, 115.f)), module, Pulse::VOCT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(48.f, 115.f)), module, Pulse::OUT_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 13.f);
		nvgTextLetterSpacing(args.vg, 4.f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "PULSE", NULL);
	}
};

Model* modelPulse = createModel<Pulse, PulseWidget>("Pulse");
