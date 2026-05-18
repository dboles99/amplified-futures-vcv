#include "plugin.hpp"

struct DroneCore : Module {
	enum ParamId {
		PITCH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PITCH_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SINE_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float phase = 0.f;

	DroneCore() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PITCH_PARAM, -3.f, 3.f, 0.f, "Pitch", " Oct");
		configInput(PITCH_INPUT, "1V/oct pitch");
		configOutput(SINE_OUTPUT, "Sine audio");
	}

	void process(const ProcessArgs& args) override {
		float pitch = params[PITCH_PARAM].getValue();
		pitch += inputs[PITCH_INPUT].getVoltage();
		float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);

		phase += freq * args.sampleTime;
		if (phase >= 1.f)
			phase -= 1.f;

		float sine = std::sin(2.f * M_PI * phase);
		outputs[SINE_OUTPUT].setVoltage(5.f * sine);
	}
};

struct DroneCoreWidget : ModuleWidget {
	DroneCoreWidget(DroneCore* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/DroneCore.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// PITCH knob — center of 3HP panel at y=44mm
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62f, 44.f)), module, DroneCore::PITCH_PARAM));

		// V/OCT input — y=98mm
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62f, 98.f)), module, DroneCore::PITCH_INPUT));

		// Sine output — y=114mm
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62f, 114.f)), module, DroneCore::SINE_OUTPUT));
	}
};

Model* modelDroneCore = createModel<DroneCore, DroneCoreWidget>("DroneCore");
