#include "plugin.hpp"

// ============================================================
// CHOKE — 4-channel cheap-mixer-as-instrument
//
// Design: Amplified Futures (paper finish, 14 HP)
//   CHANNELS: 4× IN · GAIN · TONE · MUTE (GAIN+TONE each have CV+atten)
//   MASTER:   MAIN (with CV+atten) · OUT L · OUT R
//   UTILITY:  V/OCT IN → V/OCT OUT (thru)
//
// Signal chain per channel:
//   poly input → normalize to mono → × GAIN(+CV) → TONE LP blend(+CV)
//   → mute gate → fixed auto-pan → sum to L/R bus
// Master: sum × MAIN(+CV) → tanh → L/R output
// ============================================================

struct Choke : Module {
	enum ParamId {
		// existing (IDs preserved)
		GAIN_1_PARAM, GAIN_2_PARAM, GAIN_3_PARAM, GAIN_4_PARAM,
		TONE_1_PARAM, TONE_2_PARAM, TONE_3_PARAM, TONE_4_PARAM,
		MUTE_1_PARAM, MUTE_2_PARAM, MUTE_3_PARAM, MUTE_4_PARAM,
		MAIN_PARAM,
		// appended: attenuverters
		GAIN_1_ATTEN_PARAM, GAIN_2_ATTEN_PARAM, GAIN_3_ATTEN_PARAM, GAIN_4_ATTEN_PARAM,
		TONE_1_ATTEN_PARAM, TONE_2_ATTEN_PARAM, TONE_3_ATTEN_PARAM, TONE_4_ATTEN_PARAM,
		MAIN_ATTEN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		// existing
		IN_1_INPUT, IN_2_INPUT, IN_3_INPUT, IN_4_INPUT,
		// appended
		GAIN_1_CV_INPUT, GAIN_2_CV_INPUT, GAIN_3_CV_INPUT, GAIN_4_CV_INPUT,
		TONE_1_CV_INPUT, TONE_2_CV_INPUT, TONE_3_CV_INPUT, TONE_4_CV_INPUT,
		MAIN_CV_INPUT,
		VOCT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		// existing
		OUT_L_OUTPUT, OUT_R_OUTPUT,
		// appended
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		MUTE_1_LIGHT, MUTE_2_LIGHT, MUTE_3_LIGHT, MUTE_4_LIGHT,
		LIGHTS_LEN
	};

	float toneState[4] = {};
	bool  muted[4]     = {};
	dsp::BooleanTrigger muteTrig[4];

	const float panPos[4] = { 0.f, 1.f / 3.f, 2.f / 3.f, 1.f };

	Choke() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		for (int i = 0; i < 4; i++) {
			configParam(GAIN_1_PARAM + i, 0.f, 1.5f, 0.75f,
				string::f("Ch %d gain", i + 1), "%", 0.f, 100.f);
			configParam(TONE_1_PARAM + i, 0.f, 1.f, 0.7f,
				string::f("Ch %d tone", i + 1), "%", 0.f, 100.f);
			configButton(MUTE_1_PARAM + i, string::f("Ch %d mute", i + 1));
			configInput(IN_1_INPUT + i, string::f("Ch %d", i + 1));
			configLight(MUTE_1_LIGHT + i, string::f("Ch %d mute", i + 1));
			// attenuverters + CVs
			configParam(GAIN_1_ATTEN_PARAM + i, -1.f, 1.f, 0.f,
				string::f("Ch %d gain attenuverter", i + 1));
			configParam(TONE_1_ATTEN_PARAM + i, -1.f, 1.f, 0.f,
				string::f("Ch %d tone attenuverter", i + 1));
			configInput(GAIN_1_CV_INPUT + i, string::f("Ch %d gain CV", i + 1));
			configInput(TONE_1_CV_INPUT + i, string::f("Ch %d tone CV", i + 1));
		}

		configParam(MAIN_PARAM,       0.f, 1.5f, 0.8f, "Main level",        "%", 0.f, 100.f);
		configParam(MAIN_ATTEN_PARAM, -1.f, 1.f,  0.f, "Main attenuverter");
		configOutput(OUT_L_OUTPUT, "L");
		configOutput(OUT_R_OUTPUT, "R");
		configInput(MAIN_CV_INPUT, "Main level CV");
		configInput(VOCT_INPUT,    "V/oct (thru)");
		configOutput(VOCT_OUTPUT,  "V/oct (thru)");
	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		float main = modp(MAIN_PARAM, MAIN_ATTEN_PARAM, MAIN_CV_INPUT, 0.f, 1.5f);

		float alpha = 1.f - std::exp(-2.f * M_PI * 600.f * args.sampleTime);

		float outL = 0.f;
		float outR = 0.f;

		for (int i = 0; i < 4; i++) {
			if (muteTrig[i].process(params[MUTE_1_PARAM + i].getValue() > 0.5f))
				muted[i] = !muted[i];
			lights[MUTE_1_LIGHT + i].setBrightness(muted[i] ? 1.f : 0.f);

			if (muted[i]) continue;

			int nch = std::max(1, inputs[IN_1_INPUT + i].getChannels());
			float input = 0.f;
			for (int c = 0; c < nch; c++)
				input += inputs[IN_1_INPUT + i].getVoltage(c);
			input /= nch;

			float gain = modp(GAIN_1_PARAM + i, GAIN_1_ATTEN_PARAM + i, GAIN_1_CV_INPUT + i, 0.f, 1.5f);
			input *= gain;

			// TONE: blend between LP (~600 Hz) and dry
			toneState[i] += alpha * (input - toneState[i]);
			float tone = modp(TONE_1_PARAM + i, TONE_1_ATTEN_PARAM + i, TONE_1_CV_INPUT + i, 0.f, 1.f);
			input = tone * input + (1.f - tone) * toneState[i];

			float pan   = panPos[i];
			float gainL = std::cos(pan * float(M_PI_2));
			float gainR = std::sin(pan * float(M_PI_2));

			outL += input * gainL;
			outR += input * gainR;
		}

		outL = 5.f * std::tanh(outL * main / 5.f);
		outR = 5.f * std::tanh(outR * main / 5.f);

		outputs[OUT_L_OUTPUT].setVoltage(outL);
		outputs[OUT_R_OUTPUT].setVoltage(outR);
		outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage());
	}
};

// ============================================================
// WIDGET  (14 HP)
// ============================================================

struct ChokeWidget : ModuleWidget {
	ChokeWidget(Choke* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Choke.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Channel x-centers (mm)
		const float xc[4] = { 8.89f, 26.67f, 44.45f, 62.23f };

		for (int i = 0; i < 4; i++) {
			// Satellite direction: CH1-3 right (+5mm), CH4 left (-5mm)
			float sx = (i < 3) ? xc[i] + 5.f : xc[i] - 5.f;

			// GAIN
			addParam(createParamCentered<RoundSmallBlackKnob>(
				mm2px(Vec(xc[i], 34.f)), module, Choke::GAIN_1_PARAM + i));
			addParam(createParamCentered<Trimpot>(
				mm2px(Vec(sx, 27.f)), module, Choke::GAIN_1_ATTEN_PARAM + i));
			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(sx, 41.f)), module, Choke::GAIN_1_CV_INPUT + i));

			// TONE
			addParam(createParamCentered<RoundSmallBlackKnob>(
				mm2px(Vec(xc[i], 55.f)), module, Choke::TONE_1_PARAM + i));
			addParam(createParamCentered<Trimpot>(
				mm2px(Vec(sx, 48.f)), module, Choke::TONE_1_ATTEN_PARAM + i));
			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(sx, 62.f)), module, Choke::TONE_1_CV_INPUT + i));

			// MUTE LED + button
			addChild(createLightCentered<SmallLight<RedLight>>(
				mm2px(Vec(xc[i], 72.f)), module, Choke::MUTE_1_LIGHT + i));
			addParam(createParamCentered<TL1105>(
				mm2px(Vec(xc[i], 78.f)), module, Choke::MUTE_1_PARAM + i));

			// IN jack
			addInput(createInputCentered<PJ301MPort>(
				mm2px(Vec(xc[i], 88.f)), module, Choke::IN_1_INPUT + i));
		}

		// MAIN + satellite
		addParam(createParamCentered<RoundBlackKnob>(
			mm2px(Vec(17.78f, 100.f)), module, Choke::MAIN_PARAM));
		addParam(createParamCentered<Trimpot>(
			mm2px(Vec(22.78f, 93.f)), module, Choke::MAIN_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(
			mm2px(Vec(22.78f, 109.f)), module, Choke::MAIN_CV_INPUT));

		// Bottom row: V/OCT IN · V/OCT OUT · OUT L · OUT R
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec( 8.89f, 118.f)), module, Choke::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(26.67f, 118.f)), module, Choke::VOCT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(44.45f, 118.f)), module, Choke::OUT_L_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(62.23f, 118.f)), module, Choke::OUT_R_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 13.f);
		nvgTextLetterSpacing(args.vg, 5.f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "CHOKE", NULL);
	}
};

Model* modelChoke = createModel<Choke, ChokeWidget>("Choke");
