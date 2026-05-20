#include "plugin.hpp"

// AF safety orange LED — not in the Rack SDK standard set
struct AFOrangeLight : GrayModuleLightWidget {
	AFOrangeLight() { addBaseColor(nvgRGB(0xFF, 0x4A, 0x0E)); }
};

// ============================================================
// DRONECLONE — 8-voice amplified string wall
//
// Design: Amplified Futures (concrete-finish, 22 HP)
//   All knobs have attenuverter + CV input.
//   V/OCT thru added (IN → OUT).
// ============================================================

struct DroneClone : Module {
	enum ParamId {
		// ── existing (IDs preserved) ──────────────────────────
		FUNDAMENTAL_PARAM,
		SPREAD_PARAM,
		MASS_PARAM,
		TENSION_PARAM,
		WEIGHT_PARAM,
		SHIMMER_PARAM,
		JAWARI_PARAM,
		DRIFT_PARAM,
		DECAY_PARAM,
		CHOKE_AMT_PARAM,
		CHOKE_PARAM,        // momentary button — no CV
		// ── appended: attenuverters ───────────────────────────
		FUNDAMENTAL_ATTEN_PARAM,
		SPREAD_ATTEN_PARAM,
		MASS_ATTEN_PARAM,
		TENSION_ATTEN_PARAM,
		WEIGHT_ATTEN_PARAM,
		SHIMMER_ATTEN_PARAM,
		JAWARI_ATTEN_PARAM,
		DRIFT_ATTEN_PARAM,
		DECAY_ATTEN_PARAM,
		CHOKE_AMT_ATTEN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		// ── existing ──────────────────────────────────────────
		VOCT_INPUT,
		MASS_CV_INPUT,      // moved to satellite near MASS knob
		TENS_CV_INPUT,      // moved to satellite near TENSION knob
		CHOKE_INPUT,
		RTN_INPUT,
		// ── appended: new CV inputs ───────────────────────────
		FUNDAMENTAL_CV_INPUT,
		SPREAD_CV_INPUT,
		WEIGHT_CV_INPUT,
		SHIMMER_CV_INPUT,
		JAWARI_CV_INPUT,
		DRIFT_CV_INPUT,
		DECAY_CV_INPUT,
		CHOKE_AMT_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		// appended
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		VOICE_LIGHT_0, VOICE_LIGHT_1, VOICE_LIGHT_2, VOICE_LIGHT_3,
		VOICE_LIGHT_4, VOICE_LIGHT_5, VOICE_LIGHT_6, VOICE_LIGHT_7,
		CHOKE_LIGHT,
		LIGHTS_LEN
	};

	struct Voice {
		float phase    = 0.f;
		float subPhase = 0.f;
		float level    = 0.f;
		float drift    = 0.f;
	};
	Voice voices[16][8];

	float chokeLevel  = 1.f;
	bool  gateWasHigh = false;
	dsp::SchmittTrigger btnTrig;

	float driftPhase[16][8] = {};
	float driftRate[8]      = {};

	DroneClone() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configParam(FUNDAMENTAL_PARAM, -4.f, 4.f, 0.f,
			"Fundamental", " Hz", 2.f, dsp::FREQ_C4);
		configParam(SPREAD_PARAM, 0.f, 1.f, 0.5f, "Spread", " ¢", 0.f, 1200.f);
		configParam(MASS_PARAM, 0.f, 1.f, 0.875f, "Mass", " voices", 0.f, 8.f);
		configParam(TENSION_PARAM, 0.f, 1.f, 0.5f, "Tension", "%", 0.f, 100.f);
		configParam(WEIGHT_PARAM, 0.f, 1.f, 0.3f, "Weight", "%", 0.f, 100.f);
		configParam(SHIMMER_PARAM, 0.f, 1.f, 0.4f, "Shimmer", "%", 0.f, 100.f);
		configParam(JAWARI_PARAM, 0.f, 1.f, 0.3f, "Jawari", "%", 0.f, 100.f);
		configParam(DRIFT_PARAM, 0.f, 1.f, 0.15f, "Drift", "%", 0.f, 100.f);
		configParam(DECAY_PARAM, 0.f, 1.f, 0.5f, "Decay", " s");
		configParam(CHOKE_AMT_PARAM, 0.f, 1.f, 0.8f, "Choke amount", "%", 0.f, 100.f);
		configButton(CHOKE_PARAM, "Choke");

		// Attenuverters
		for (int i = FUNDAMENTAL_ATTEN_PARAM; i < PARAMS_LEN; i++)
			configParam(i, -1.f, 1.f, 0.f, "Attenuverter");

		configInput(VOCT_INPUT,           "V/oct pitch");
		configInput(MASS_CV_INPUT,        "Mass CV (0–10 V)");
		configInput(TENS_CV_INPUT,        "Tension CV (0–10 V)");
		configInput(CHOKE_INPUT,          "Choke gate");
		configInput(RTN_INPUT,            "Feedback return");
		configInput(FUNDAMENTAL_CV_INPUT, "Fundamental CV");
		configInput(SPREAD_CV_INPUT,      "Spread CV");
		configInput(WEIGHT_CV_INPUT,      "Weight CV");
		configInput(SHIMMER_CV_INPUT,     "Shimmer CV");
		configInput(JAWARI_CV_INPUT,      "Jawari CV");
		configInput(DRIFT_CV_INPUT,       "Drift CV");
		configInput(DECAY_CV_INPUT,       "Decay CV");
		configInput(CHOKE_AMT_CV_INPUT,   "Choke amount CV");

		configOutput(OUT_OUTPUT,  "Drone wall audio");
		configOutput(VOCT_OUTPUT, "V/oct thru");

		for (int i = 0; i < 8; i++)
			configLight(VOICE_LIGHT_0 + i, string::f("Voice %d", i + 1));
		configLight(CHOKE_LIGHT, "Choke");

		const float rates[8] = {0.31f, 0.47f, 0.61f, 0.79f, 0.89f, 1.03f, 1.19f, 1.37f};
		for (int i = 0; i < 8; i++)
			driftRate[i] = rates[i] * 0.003f;
	}

	// CV-attenuated mono param helper
	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	// CV-attenuated poly param helper (per channel c)
	float modpoly(int param, int atten, int cv, int c, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getPolyVoltage(c) / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		// Global params (CV mono)
		float spread   = modp(SPREAD_PARAM,   SPREAD_ATTEN_PARAM,    SPREAD_CV_INPUT,    0.f, 1.f);
		float weight   = modp(WEIGHT_PARAM,   WEIGHT_ATTEN_PARAM,    WEIGHT_CV_INPUT,    0.f, 1.f);
		float shimmer  = modp(SHIMMER_PARAM,  SHIMMER_ATTEN_PARAM,   SHIMMER_CV_INPUT,   0.f, 1.f);
		float jawari   = modp(JAWARI_PARAM,   JAWARI_ATTEN_PARAM,    JAWARI_CV_INPUT,    0.f, 1.f);
		float drift    = modp(DRIFT_PARAM,    DRIFT_ATTEN_PARAM,     DRIFT_CV_INPUT,     0.f, 1.f);
		float chokeAmt = modp(CHOKE_AMT_PARAM,CHOKE_AMT_ATTEN_PARAM, CHOKE_AMT_CV_INPUT, 0.f, 1.f);
		float fundKnob = modp(FUNDAMENTAL_PARAM, FUNDAMENTAL_ATTEN_PARAM, FUNDAMENTAL_CV_INPUT, -4.f, 4.f);

		float decayV   = modp(DECAY_PARAM, DECAY_ATTEN_PARAM, DECAY_CV_INPUT, 0.f, 1.f);
		float decayTime = 0.02f * std::pow(400.f, decayV);

		// Choke (global)
		bool btnDown  = params[CHOKE_PARAM].getValue() > 0.5f;
		bool gateHigh = inputs[CHOKE_INPUT].getVoltage() > 2.f;
		bool choking  = btnDown || gateHigh;

		float chokeTarget = choking ? (1.f - chokeAmt) : 1.f;
		float chokeRate   = 1.f / (decayTime * args.sampleRate);
		if (choking && chokeLevel > chokeTarget)
			chokeLevel -= chokeRate;
		else if (!choking && chokeLevel < 1.f)
			chokeLevel += chokeRate * 2.f;
		chokeLevel = clamp(chokeLevel, 0.f, 1.f);
		lights[CHOKE_LIGHT].setBrightness(choking ? 1.f : 0.f);

		int channels = std::max(1, inputs[VOCT_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);
		outputs[VOCT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c++) {
			float basePitch = fundKnob + inputs[VOCT_INPUT].getVoltage(c);
			float baseFreq  = dsp::FREQ_C4 * dsp::exp2_taylor5(basePitch);

			float massKnob = modpoly(MASS_PARAM, MASS_ATTEN_PARAM, MASS_CV_INPUT, c, 0.f, 1.f);
			float massFloat = massKnob * 8.f;

			float tension = modpoly(TENSION_PARAM, TENSION_ATTEN_PARAM, TENS_CV_INPUT, c, 0.f, 1.f);

			float output = 0.f;

			for (int i = 0; i < 8; i++) {
				float targetLevel = (static_cast<float>(i) < massFloat) ? 1.f : 0.f;
				if (massFloat > static_cast<float>(i) && massFloat < static_cast<float>(i) + 1.f)
					targetLevel = massFloat - static_cast<float>(i);
				voices[c][i].level += (targetLevel - voices[c][i].level) * 4e-4f;

				if (voices[c][i].level < 1e-4f) {
					if (c == 0) lights[VOICE_LIGHT_0 + i].setBrightness(0.f);
					continue;
				}

				float normalizedPos = (i / 7.f) - 0.5f;
				float detuneCents   = normalizedPos * spread * 1200.f;
				float detuneFreq    = baseFreq * dsp::exp2_taylor5(detuneCents / 1200.f);

				driftPhase[c][i] += driftRate[i];
				if (driftPhase[c][i] >= 1.f) driftPhase[c][i] -= 1.f;
				detuneFreq *= 1.f + drift * 0.008f * std::sin(2.f * M_PI * driftPhase[c][i]);

				voices[c][i].phase += detuneFreq * args.sampleTime;
				if (voices[c][i].phase >= 1.f) voices[c][i].phase -= 1.f;

				float p = voices[c][i].phase;
				float fundamental_wave = std::sin(2.f * M_PI * p);
				float toneSignal = fundamental_wave;

				if (tension > 0.f) {
					toneSignal += tension * 0.50f * std::sin(4.f * M_PI * p);
					toneSignal += tension * 0.33f * std::sin(6.f * M_PI * p);
					toneSignal += tension * 0.25f * std::sin(8.f * M_PI * p);
					toneSignal /= 1.f + tension * 1.08f;
				}
				if (shimmer > 0.f) {
					toneSignal += shimmer * 0.12f * std::sin(10.f * M_PI * p);
					toneSignal += shimmer * 0.08f * std::sin(14.f * M_PI * p);
				}
				if (jawari > 0.f) {
					float buzz = std::max(0.f, (float)std::sin(2.f * M_PI * (p + 0.01f)));
					toneSignal = toneSignal * (1.f - jawari * 0.35f) + buzz * jawari * 0.35f;
				}

				float subSignal = 0.f;
				if (weight > 0.f) {
					voices[c][i].subPhase += detuneFreq * 0.25f * args.sampleTime;
					if (voices[c][i].subPhase >= 1.f) voices[c][i].subPhase -= 1.f;
					subSignal = weight * 0.4f * std::sin(2.f * M_PI * voices[c][i].subPhase);
				}

				output += (toneSignal + subSignal) * voices[c][i].level;

				if (c == 0) {
					float ledBright = voices[c][i].level * (0.5f + 0.5f * fundamental_wave);
					lights[VOICE_LIGHT_0 + i].setSmoothBrightness(ledBright, args.sampleTime);
				}
			}

			output /= 8.f;
			output *= chokeLevel;

			if (inputs[RTN_INPUT].isConnected())
				output += inputs[RTN_INPUT].getPolyVoltage(c) * 0.1f;

			outputs[OUT_OUTPUT].setVoltage(5.f * std::tanh(output), c);
			outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage(c), c);
		}
	}
};

// ============================================================
// WIDGET
// ============================================================

struct DroneCloneWidget : ModuleWidget {
	DroneCloneWidget(DroneClone* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/DroneClone.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Voice LEDs
		const float ledY = 12.7f, ledX0 = 12.4f, ledSpacing = 12.4f;
		for (int i = 0; i < 8; i++)
			addChild(createLightCentered<TinyLight<AFOrangeLight>>(
				mm2px(Vec(ledX0 + i * ledSpacing, ledY)),
				module, DroneClone::VOICE_LIGHT_0 + i));

		// TUNING row (y=28mm) — main knobs + satellites to upper-right
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(27.9f, 28.f)), module, DroneClone::FUNDAMENTAL_PARAM));
		addParam(createParamCentered<Trimpot>(          mm2px(Vec(40.f,  21.f)), module, DroneClone::FUNDAMENTAL_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(       mm2px(Vec(40.f,  35.f)), module, DroneClone::FUNDAMENTAL_CV_INPUT));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(83.8f, 28.f)), module, DroneClone::SPREAD_PARAM));
		addParam(createParamCentered<Trimpot>(          mm2px(Vec(96.f,  21.f)), module, DroneClone::SPREAD_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(       mm2px(Vec(96.f,  35.f)), module, DroneClone::SPREAD_CV_INPUT));

		// MASS/TENSION/WEIGHT row (y=52mm) — satellites to right
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(18.6f, 52.f)), module, DroneClone::MASS_PARAM));
		addParam(createParamCentered<Trimpot>(       mm2px(Vec(27.f,  44.f)), module, DroneClone::MASS_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(    mm2px(Vec(27.f,  60.f)), module, DroneClone::MASS_CV_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(55.9f, 52.f)), module, DroneClone::TENSION_PARAM));
		addParam(createParamCentered<Trimpot>(       mm2px(Vec(65.f,  44.f)), module, DroneClone::TENSION_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(    mm2px(Vec(65.f,  60.f)), module, DroneClone::TENS_CV_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(93.1f, 52.f)), module, DroneClone::WEIGHT_PARAM));
		addParam(createParamCentered<Trimpot>(       mm2px(Vec(101.5f,44.f)), module, DroneClone::WEIGHT_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(    mm2px(Vec(101.5f,60.f)), module, DroneClone::WEIGHT_CV_INPUT));

		// SHIMMER/JAWARI/DRIFT row (y=66mm) — satellites to LEFT (avoids row-above conflict)
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.6f, 66.f)), module, DroneClone::SHIMMER_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(10.f,  59.f)), module, DroneClone::SHIMMER_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(10.f,  73.f)), module, DroneClone::SHIMMER_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.9f, 66.f)), module, DroneClone::JAWARI_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(46.f,  59.f)), module, DroneClone::JAWARI_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(46.f,  73.f)), module, DroneClone::JAWARI_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(93.1f, 66.f)), module, DroneClone::DRIFT_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(83.f,  59.f)), module, DroneClone::DRIFT_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(83.f,  73.f)), module, DroneClone::DRIFT_CV_INPUT));

		// COLLAPSE row (y=85mm)
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(27.9f, 85.f)), module, DroneClone::DECAY_PARAM));
		addParam(createParamCentered<Trimpot>(          mm2px(Vec(40.f,  78.f)), module, DroneClone::DECAY_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(       mm2px(Vec(40.f,  92.f)), module, DroneClone::DECAY_CV_INPUT));

		addParam(createParamCentered<TL1105>(  mm2px(Vec(55.9f, 85.f)), module, DroneClone::CHOKE_PARAM));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(55.9f, 77.f)), module, DroneClone::CHOKE_LIGHT));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(83.8f, 85.f)), module, DroneClone::CHOKE_AMT_PARAM));
		addParam(createParamCentered<Trimpot>(          mm2px(Vec(96.f,  78.f)), module, DroneClone::CHOKE_AMT_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(       mm2px(Vec(96.f,  92.f)), module, DroneClone::CHOKE_AMT_CV_INPUT));

		// CV / IO row (y=101mm)
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(14.f,  101.f)), module, DroneClone::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(37.f,  101.f)), module, DroneClone::VOCT_OUTPUT));
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(97.5f, 101.f)), module, DroneClone::CHOKE_INPUT));

		// OUT / RTN row (y=116mm)
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(75.f, 116.f)), module, DroneClone::OUT_OUTPUT));
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(95.f, 116.f)), module, DroneClone::RTN_INPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 15.f);
		nvgTextLetterSpacing(args.vg, 3.f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 13.f, "DRONECLONE", NULL);
	}
};

Model* modelDroneClone = createModel<DroneClone, DroneCloneWidget>("DroneClone");
