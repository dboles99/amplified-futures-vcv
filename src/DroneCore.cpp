#include "plugin.hpp"

struct DroneCore : Module {
	enum ParamId {
		PITCH_PARAM,
		PITCH_ATTN_PARAM,
		DETUNE_PARAM,
		DETUNE_ATTN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PITCH_INPUT,
		PITCH_MOD_INPUT,
		DETUNE_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SINE_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float phaseA = 0.f;
	float phaseB = 0.25f;

	DroneCore() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PITCH_PARAM, -3.f, 3.f, 0.f, "Pitch", " Oct");
		configParam(PITCH_ATTN_PARAM, -1.f, 1.f, 0.f, "Pitch mod depth");
		configParam(DETUNE_PARAM, 0.f, 100.f, 0.f, "Detune", " cents");
		configParam(DETUNE_ATTN_PARAM, -1.f, 1.f, 0.f, "Detune mod depth");
		configInput(PITCH_INPUT, "1V/oct pitch");
		configInput(PITCH_MOD_INPUT, "Pitch modulation");
		configInput(DETUNE_CV_INPUT, "Detune CV");
		configOutput(SINE_OUTPUT, "Sine audio");
	}

	void process(const ProcessArgs& args) override {
		// Pitch: base knob + V/oct + attenuated mod input
		// Mod scaled so 5V × attn=1 = 1 octave
		float pitch = params[PITCH_PARAM].getValue();
		pitch += inputs[PITCH_INPUT].getVoltage();
		if (inputs[PITCH_MOD_INPUT].isConnected())
			pitch += inputs[PITCH_MOD_INPUT].getVoltage() * params[PITCH_ATTN_PARAM].getValue() / 5.f;

		float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);

		// Detune: base knob + attenuated CV
		// 5V × attn=1 = 50 cents swing
		float detune = params[DETUNE_PARAM].getValue();
		if (inputs[DETUNE_CV_INPUT].isConnected())
			detune += inputs[DETUNE_CV_INPUT].getVoltage() * params[DETUNE_ATTN_PARAM].getValue() / 5.f * 50.f;
		detune = clamp(detune, 0.f, 100.f);

		// Two internal voices split evenly around center pitch
		float detuneOct = detune / 2400.f;
		float freqA = freq * std::pow(2.f,  detuneOct);
		float freqB = freq * std::pow(2.f, -detuneOct);

		phaseA += freqA * args.sampleTime;
		if (phaseA >= 1.f) phaseA -= 1.f;

		phaseB += freqB * args.sampleTime;
		if (phaseB >= 1.f) phaseB -= 1.f;

		float sineA = std::sin(2.f * M_PI * phaseA);
		float sineB = std::sin(2.f * M_PI * phaseB);

		outputs[SINE_OUTPUT].setVoltage(5.f * (sineA + sineB) * 0.5f);
	}
};

struct DroneCoreWidget : ModuleWidget {
	DroneCoreWidget(DroneCore* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/DroneCore.svg")));

		// 6HP module — screws at HP-2 and HP-5 centers
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Left col x=7.62mm  Right col x=22.86mm

		// PITCH section
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62f, 38.f)),  module, DroneCore::PITCH_PARAM));
		addInput(createInputCentered<PJ301MPort>(    mm2px(Vec(22.86f, 30.f)), module, DroneCore::PITCH_MOD_INPUT));
		addParam(createParamCentered<Trimpot>(       mm2px(Vec(22.86f, 46.f)), module, DroneCore::PITCH_ATTN_PARAM));

		// DETUNE section
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62f, 70.f)),  module, DroneCore::DETUNE_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(22.86f, 62.f)), module, DroneCore::DETUNE_CV_INPUT));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(22.86f, 78.f)), module, DroneCore::DETUNE_ATTN_PARAM));

		// I/O
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(7.62f,  95.f)), module, DroneCore::PITCH_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86f, 107.f)), module, DroneCore::SINE_OUTPUT));
	}
};

Model* modelDroneCore = createModel<DroneCore, DroneCoreWidget>("DroneCore");
