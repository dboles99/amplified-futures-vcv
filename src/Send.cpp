#include "plugin.hpp"

// ============================================================
// SEND — 2×2 cross-send feedback routing matrix
//
// Design: Amplified Futures (dark finish, 12 HP)
//   All knobs have attenuverter + CV. V/OCT thru added.
//
// Signal flow (unchanged):
//   OUT A = IN A + (B→A × IN B) + (C→A × cBus)  [tanh]
//   OUT B = A→B × IN A
//   cBus  = A→C × IN A  [1-sample delay]
// ============================================================

struct Send : Module {
	enum ParamId {
		// existing (IDs preserved)
		A_TO_B_PARAM,
		B_TO_A_PARAM,
		A_TO_C_PARAM,
		C_TO_A_PARAM,
		// appended: attenuverters
		A_TO_B_ATTEN_PARAM,
		B_TO_A_ATTEN_PARAM,
		A_TO_C_ATTEN_PARAM,
		C_TO_A_ATTEN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		// existing
		IN_A_INPUT,
		IN_B_INPUT,
		// appended
		A_TO_B_CV_INPUT,
		B_TO_A_CV_INPUT,
		A_TO_C_CV_INPUT,
		C_TO_A_CV_INPUT,
		VOCT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		// existing
		OUT_A_OUTPUT,
		OUT_B_OUTPUT,
		// appended
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float cBus[16] = {};

	Send() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(A_TO_B_PARAM, 0.f, 1.f, 0.5f, "A→B send",            "%", 0.f, 100.f);
		configParam(B_TO_A_PARAM, 0.f, 1.f, 0.5f, "B→A return",          "%", 0.f, 100.f);
		configParam(A_TO_C_PARAM, 0.f, 1.f, 0.f,  "A→C feedback depth",  "%", 0.f, 100.f);
		configParam(C_TO_A_PARAM, 0.f, 1.f, 0.f,  "C→A feedback return", "%", 0.f, 100.f);
		configParam(A_TO_B_ATTEN_PARAM, -1.f, 1.f, 0.f, "A→B attenuverter");
		configParam(B_TO_A_ATTEN_PARAM, -1.f, 1.f, 0.f, "B→A attenuverter");
		configParam(A_TO_C_ATTEN_PARAM, -1.f, 1.f, 0.f, "A→C attenuverter");
		configParam(C_TO_A_ATTEN_PARAM, -1.f, 1.f, 0.f, "C→A attenuverter");
		configInput(IN_A_INPUT,       "A");
		configInput(IN_B_INPUT,       "B");
		configInput(A_TO_B_CV_INPUT,  "A→B CV");
		configInput(B_TO_A_CV_INPUT,  "B→A CV");
		configInput(A_TO_C_CV_INPUT,  "A→C CV");
		configInput(C_TO_A_CV_INPUT,  "C→A CV");
		configInput(VOCT_INPUT,       "V/oct (thru)");
		configOutput(OUT_A_OUTPUT,    "A main");
		configOutput(OUT_B_OUTPUT,    "B send");
		configOutput(VOCT_OUTPUT,     "V/oct (thru)");
	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		int channels = std::max({1, inputs[IN_A_INPUT].getChannels(),
		                            inputs[IN_B_INPUT].getChannels()});
		outputs[OUT_A_OUTPUT].setChannels(channels);
		outputs[OUT_B_OUTPUT].setChannels(channels);

		float aToB = modp(A_TO_B_PARAM, A_TO_B_ATTEN_PARAM, A_TO_B_CV_INPUT, 0.f, 1.f);
		float bToA = modp(B_TO_A_PARAM, B_TO_A_ATTEN_PARAM, B_TO_A_CV_INPUT, 0.f, 1.f);
		float aToC = modp(A_TO_C_PARAM, A_TO_C_ATTEN_PARAM, A_TO_C_CV_INPUT, 0.f, 1.f);
		float cToA = modp(C_TO_A_PARAM, C_TO_A_ATTEN_PARAM, C_TO_A_CV_INPUT, 0.f, 1.f);

		for (int c = 0; c < channels; c++) {
			float inA = inputs[IN_A_INPUT].getPolyVoltage(c);
			float inB = inputs[IN_B_INPUT].getPolyVoltage(c);

			float outA = inA + bToA * inB + cToA * cBus[c];
			float outB = aToB * inA;
			cBus[c] = aToC * inA;

			outA = 5.f * std::tanh(outA / 5.f);

			outputs[OUT_A_OUTPUT].setVoltage(outA, c);
			outputs[OUT_B_OUTPUT].setVoltage(outB, c);
		}

		outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage());
	}
};

// ============================================================
// WIDGET  (12 HP)
// ============================================================

struct SendWidget : ModuleWidget {
	SendWidget(Send* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Send.svg")));

		// 12HP screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Left col x=15.24mm   Right col x=45.72mm
		// Satellites at +7mm to the right of each knob

		// A→B and B→A (top pair)
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 48.f)), module, Send::A_TO_B_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(22.24f, 41.f)), module, Send::A_TO_B_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(22.24f, 55.f)), module, Send::A_TO_B_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(45.72f, 48.f)), module, Send::B_TO_A_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(52.72f, 41.f)), module, Send::B_TO_A_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(52.72f, 55.f)), module, Send::B_TO_A_CV_INPUT));

		// A→C and C→A (bottom pair)
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 70.f)), module, Send::A_TO_C_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(22.24f, 63.f)), module, Send::A_TO_C_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(22.24f, 77.f)), module, Send::A_TO_C_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(45.72f, 70.f)), module, Send::C_TO_A_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(52.72f, 63.f)), module, Send::C_TO_A_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(52.72f, 77.f)), module, Send::C_TO_A_CV_INPUT));

		// Audio I/O
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(15.24f, 90.f)), module, Send::IN_A_INPUT));
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(45.72f, 90.f)), module, Send::IN_B_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24f, 104.f)), module, Send::OUT_A_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.72f, 104.f)), module, Send::OUT_B_OUTPUT));

		// V/OCT thru
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(15.24f, 116.f)), module, Send::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(45.72f, 116.f)), module, Send::VOCT_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 13.f);
		nvgTextLetterSpacing(args.vg, 6.f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "SEND", NULL);
	}
};

Model* modelSend = createModel<Send, SendWidget>("Send");
