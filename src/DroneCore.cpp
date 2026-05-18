#include "plugin.hpp"

struct DroneCore : Module {
	enum ParamId {
		PITCH_PARAM,
		DETUNE_PARAM,
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

	// Two internal voices for detuned beating
	float phaseA = 0.f;
	float phaseB = 0.25f; // start offset so they don't sum to zero at t=0

	DroneCore() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PITCH_PARAM, -3.f, 3.f, 0.f, "Pitch", " Oct");
		configParam(DETUNE_PARAM, 0.f, 100.f, 0.f, "Detune", " cents");
		configInput(PITCH_INPUT, "1V/oct pitch");
		configOutput(SINE_OUTPUT, "Sine audio");
	}

	void process(const ProcessArgs& args) override {
		float pitch = params[PITCH_PARAM].getValue();
		pitch += inputs[PITCH_INPUT].getVoltage();
		float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);

		// Detune: split evenly above and below center pitch
		// 100 cents = 1 semitone; expressed as octave fraction for pow(2, x)
		float detuneOct = params[DETUNE_PARAM].getValue() / 2400.f;
		float freqA = freq * std::pow(2.f,  detuneOct);
		float freqB = freq * std::pow(2.f, -detuneOct);

		phaseA += freqA * args.sampleTime;
		if (phaseA >= 1.f) phaseA -= 1.f;

		phaseB += freqB * args.sampleTime;
		if (phaseB >= 1.f) phaseB -= 1.f;

		float sineA = std::sin(2.f * M_PI * phaseA);
		float sineB = std::sin(2.f * M_PI * phaseB);

		// Mix and normalize — two voices, keep peak at +/-5V
		outputs[SINE_OUTPUT].setVoltage(5.f * (sineA + sineB) * 0.5f);
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

		// PITCH knob — y=36mm
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62f, 36.f)), module, DroneCore::PITCH_PARAM));

		// DETUNE knob — y=60mm
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62f, 60.f)), module, DroneCore::DETUNE_PARAM));

		// V/OCT input — y=98mm
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62f, 98.f)), module, DroneCore::PITCH_INPUT));

		// Sine output — y=114mm
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62f, 114.f)), module, DroneCore::SINE_OUTPUT));
	}
};

Model* modelDroneCore = createModel<DroneCore, DroneCoreWidget>("DroneCore");
