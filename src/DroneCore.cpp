#include "plugin.hpp"

// ============================================================
// DRONECORE — two-voice detuned oscillator
//
// Design: Amplified Futures (steel finish, 8 HP)
//   PITCH · DETUNE · TIMBRE — each with attenuverter + CV
//   V/OCT IN (poly) · V/OCT OUT (thru) · SINE OUT (poly)
// ============================================================

struct DroneCore : Module {
	enum ParamId {
		PITCH_PARAM,
		PITCH_ATTN_PARAM,
		DETUNE_PARAM,
		DETUNE_ATTN_PARAM,
		TIMBRE_PARAM,
		// appended — preserves existing patch IDs
		TIMBRE_ATTN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PITCH_INPUT,
		PITCH_MOD_INPUT,
		DETUNE_CV_INPUT,
		// appended
		TIMBRE_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SINE_OUTPUT,
		// appended
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float phaseA[16] = {};
	float phaseB[16] = {};

	DroneCore() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int c = 0; c < 16; c++) phaseB[c] = 0.25f;

		configParam(PITCH_PARAM,      -1.667f, 2.333f, 0.f,  "Pitch",          " Oct");
		configParam(PITCH_ATTN_PARAM, -1.f,    1.f,    0.f,  "Pitch mod depth");
		configParam(DETUNE_PARAM,      0.f,    100.f,  0.f,  "Detune",         " cents");
		configParam(DETUNE_ATTN_PARAM,-1.f,    1.f,    0.f,  "Detune mod depth");
		configParam(TIMBRE_PARAM,      0.f,    1.f,    0.f,  "Timbre",         "%", 0.f, 100.f);
		configParam(TIMBRE_ATTN_PARAM,-1.f,    1.f,    0.f,  "Timbre attenuverter");
		getParamQuantity(TIMBRE_PARAM)->description = "0 = pure sine, 1 = harmonics 2-4 (third-bridge harmonic stack)";

		configInput(PITCH_INPUT,    "V/oct pitch");
		configInput(PITCH_MOD_INPUT,"Pitch modulation CV");
		configInput(DETUNE_CV_INPUT,"Detune CV");
		configInput(TIMBRE_CV_INPUT,"Timbre CV");
		configOutput(SINE_OUTPUT,   "Sine audio (poly)");
		configOutput(VOCT_OUTPUT,   "V/oct thru (poly)");
	}

	void process(const ProcessArgs& args) override {
		int channels = std::max(1, inputs[PITCH_INPUT].getChannels());
		outputs[SINE_OUTPUT].setChannels(channels);
		outputs[VOCT_OUTPUT].setChannels(channels);

		// Timbre is global (same for all poly channels)
		float timbre = params[TIMBRE_PARAM].getValue();
		if (inputs[TIMBRE_CV_INPUT].isConnected())
			timbre += params[TIMBRE_ATTN_PARAM].getValue() * inputs[TIMBRE_CV_INPUT].getVoltage() / 5.f;
		timbre = clamp(timbre, 0.f, 1.f);
		float norm = 1.f + timbre * 1.08f;

		for (int c = 0; c < channels; c++) {
			float pitch = params[PITCH_PARAM].getValue();
			pitch += inputs[PITCH_INPUT].getVoltage(c);
			if (inputs[PITCH_MOD_INPUT].isConnected())
				pitch += inputs[PITCH_MOD_INPUT].getPolyVoltage(c) * params[PITCH_ATTN_PARAM].getValue() / 5.f;
			pitch = clamp(pitch, -1.667f, 2.333f);
			float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);

			float detune = params[DETUNE_PARAM].getValue();
			if (inputs[DETUNE_CV_INPUT].isConnected())
				detune += inputs[DETUNE_CV_INPUT].getPolyVoltage(c) * params[DETUNE_ATTN_PARAM].getValue() / 5.f * 50.f;
			detune = clamp(detune, 0.f, 100.f);

			float detuneOct = detune / 2400.f;
			float freqA = freq * std::pow(2.f,  detuneOct);
			float freqB = freq * std::pow(2.f, -detuneOct);

			phaseA[c] += freqA * args.sampleTime;
			if (phaseA[c] >= 1.f) phaseA[c] -= 1.f;
			phaseB[c] += freqB * args.sampleTime;
			if (phaseB[c] >= 1.f) phaseB[c] -= 1.f;

			float sineA = (std::sin(2.f * M_PI * phaseA[c])
			             + timbre * 0.5f  * std::sin(4.f * M_PI * phaseA[c])
			             + timbre * 0.33f * std::sin(6.f * M_PI * phaseA[c])
			             + timbre * 0.25f * std::sin(8.f * M_PI * phaseA[c])) / norm;
			float sineB = (std::sin(2.f * M_PI * phaseB[c])
			             + timbre * 0.5f  * std::sin(4.f * M_PI * phaseB[c])
			             + timbre * 0.33f * std::sin(6.f * M_PI * phaseB[c])
			             + timbre * 0.25f * std::sin(8.f * M_PI * phaseB[c])) / norm;

			outputs[SINE_OUTPUT].setVoltage(5.f * (sineA + sineB) * 0.5f, c);
			outputs[VOCT_OUTPUT].setVoltage(inputs[PITCH_INPUT].getVoltage(c), c);
		}
	}
};

// ============================================================
// WIDGET  (8 HP)
// ============================================================

struct DroneCoreWidget : ModuleWidget {
	DroneCoreWidget(DroneCore* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/DroneCore.svg")));

		// 8HP — 4 screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Left col x=10.16mm  Right col x=30.48mm

		// PITCH section
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16f, 38.f)), module, DroneCore::PITCH_PARAM));
		addInput(createInputCentered<PJ301MPort>(    mm2px(Vec(30.48f, 28.f)), module, DroneCore::PITCH_MOD_INPUT));
		addParam(createParamCentered<Trimpot>(       mm2px(Vec(30.48f, 48.f)), module, DroneCore::PITCH_ATTN_PARAM));

		// DETUNE section
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.16f, 65.f)), module, DroneCore::DETUNE_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(30.48f, 58.f)), module, DroneCore::DETUNE_CV_INPUT));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(30.48f, 74.f)), module, DroneCore::DETUNE_ATTN_PARAM));

		// TIMBRE section
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.16f, 91.f)), module, DroneCore::TIMBRE_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(30.48f, 84.f)), module, DroneCore::TIMBRE_CV_INPUT));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(30.48f, 100.f)), module, DroneCore::TIMBRE_ATTN_PARAM));

		// I/O row
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(10.16f, 113.f)), module, DroneCore::PITCH_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32f, 113.f)), module, DroneCore::VOCT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.48f, 113.f)), module, DroneCore::SINE_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 11.f);
		nvgTextLetterSpacing(args.vg, 1.5f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "DRONECORE", NULL);
	}
};

Model* modelDroneCore = createModel<DroneCore, DroneCoreWidget>("DroneCore");
