// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include "dsp/DroneEngine.hpp"
#include "dsp/AfTuning.hpp"

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


	// One wall per polyphonic channel. Each engine owns its own voices and
	// drift phases; the module keeps only what is global or host-facing.
	af::DroneEngine engines[16];
	float engineSr = 0.f;
	float chokeLevel  = 1.f;

	// Beat rate between ADJACENT voices at the current pitch and SPREAD,
	// published for the context menu. Not the rate between the extremes:
	// at SPREAD 1.0 the outermost pair is 1200 cents apart, which is a
	// different note rather than a beat, and reporting it would tell the
	// player nothing about the shimmer they can actually hear.
	float beatRateHz_ = 0.f;
	bool  gateWasHigh = false;
	dsp::SchmittTrigger btnTrig;


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

		// Bypass passes audio through rather than muting it. Without this, bypassing
		// the module drops its outputs to zero and the patch goes quiet.
		configBypass(VOCT_INPUT, VOCT_OUTPUT);

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
		if (args.sampleRate != engineSr) {
			engineSr = args.sampleRate;
			for (auto& e : engines)
				e.setSampleRate(engineSr);
		}

		// Global, mono CV.
		af::DroneEngine::Params p;
		p.spread  = modp(SPREAD_PARAM,  SPREAD_ATTEN_PARAM,  SPREAD_CV_INPUT,  0.f, 1.f);
		p.weight  = modp(WEIGHT_PARAM,  WEIGHT_ATTEN_PARAM,  WEIGHT_CV_INPUT,  0.f, 1.f);
		p.shimmer = modp(SHIMMER_PARAM, SHIMMER_ATTEN_PARAM, SHIMMER_CV_INPUT, 0.f, 1.f);
		p.jawari  = modp(JAWARI_PARAM,  JAWARI_ATTEN_PARAM,  JAWARI_CV_INPUT,  0.f, 1.f);
		p.drift   = modp(DRIFT_PARAM,   DRIFT_ATTEN_PARAM,   DRIFT_CV_INPUT,   0.f, 1.f);

		const float chokeAmt = modp(CHOKE_AMT_PARAM, CHOKE_AMT_ATTEN_PARAM,
		                            CHOKE_AMT_CV_INPUT, 0.f, 1.f);
		const float fundKnob = modp(FUNDAMENTAL_PARAM, FUNDAMENTAL_ATTEN_PARAM,
		                            FUNDAMENTAL_CV_INPUT, -4.f, 4.f);
		const float decayV   = modp(DECAY_PARAM, DECAY_ATTEN_PARAM,
		                            DECAY_CV_INPUT, 0.f, 1.f);
		const float decayTime = 0.02f * std::pow(400.f, decayV);

		// Choke is global rather than per voice, so it stays in the module.
		const bool btnDown  = params[CHOKE_PARAM].getValue() > 0.5f;
		const bool gateHigh = inputs[CHOKE_INPUT].getVoltage() > 2.f;
		const bool choking  = btnDown || gateHigh;

		const float chokeTarget = choking ? (1.f - chokeAmt) : 1.f;
		const float chokeRate   = 1.f / (decayTime * args.sampleRate);
		if (choking && chokeLevel > chokeTarget)
			chokeLevel -= chokeRate;
		else if (!choking && chokeLevel < 1.f)
			chokeLevel += chokeRate * 2.f;
		chokeLevel = clamp(chokeLevel, 0.f, 1.f);
		lights[CHOKE_LIGHT].setBrightness(choking ? 1.f : 0.f);

		const int channels = std::max(1, inputs[VOCT_INPUT].getChannels());
		outputs[OUT_OUTPUT].setChannels(channels);
		outputs[VOCT_OUTPUT].setChannels(channels);

		for (int c = 0; c < channels; c++) {
			const float basePitch = fundKnob + inputs[VOCT_INPUT].getVoltage(c);

			// Channel 0 only, and outside the voice loop: this is a readout,
			// not a signal, and writing it per voice would leave it holding
			// whichever voice happened to run last. Beat rate is linear in
			// frequency, so the same cents shimmers in the bass and roughens
			// in the treble - AfTuning.hpp.
			if (c == 0) {
				const float baseFreq = dsp::FREQ_C4 * dsp::exp2_taylor5(basePitch);
				const float adjacentCents = p.spread * 1200.f / 7.f;
				beatRateHz_ = af::tuning::beatHz(baseFreq, adjacentCents);
			}

			// MASS and TENSION are per channel, so they are read inside the loop.
			p.mass = modpoly(MASS_PARAM, MASS_ATTEN_PARAM, MASS_CV_INPUT, c, 0.f, 1.f);
			p.tension = modpoly(TENSION_PARAM, TENSION_ATTEN_PARAM, TENS_CV_INPUT,
			                    c, 0.f, 1.f);

			float output = engines[c].process(basePitch, p.mass * 8.f, p);
			output *= chokeLevel;

			if (inputs[RTN_INPUT].isConnected())
				output += inputs[RTN_INPUT].getPolyVoltage(c) * 0.1f;

			outputs[OUT_OUTPUT].setVoltage(5.f * std::tanh(output), c);
			outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage(c), c);

			// Lights are the host's business, so they read the engine rather
			// than living inside it.
			if (c == 0) {
				for (int i = 0; i < af::DroneEngine::VOICES; i++) {
					const float lvl = engines[c].voiceLevel(i);
					if (lvl < 1e-4f) {
						lights[VOICE_LIGHT_0 + i].setBrightness(0.f);
					} else {
						lights[VOICE_LIGHT_0 + i].setSmoothBrightness(
							lvl * (0.5f + 0.5f * engines[c].voiceWave(i)),
							args.sampleTime);
					}
				}
			}
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


		// Voice LEDs
		const float ledY = 12.7f, ledX0 = 14.65f, ledSpacing = 14.65f;   // scaled with the 22->26 HP widening
		for (int i = 0; i < 8; i++)
			addChild(createLightCentered<TinyLight<AFOrangeLight>>(
				mm2px(Vec(ledX0 + i * ledSpacing, ledY)),
				module, DroneClone::VOICE_LIGHT_0 + i));

		// TUNING row (y=28mm) — main knobs + satellites to upper-right
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(32.97f, 28.f)), module, DroneClone::FUNDAMENTAL_PARAM));
		addParam(createParamCentered<Trimpot>(          mm2px(Vec(47.27f,  21.f)), module, DroneClone::FUNDAMENTAL_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(       mm2px(Vec(47.27f,  35.f)), module, DroneClone::FUNDAMENTAL_CV_INPUT));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(99.04f, 28.f)), module, DroneClone::SPREAD_PARAM));
		addParam(createParamCentered<Trimpot>(          mm2px(Vec(113.5f,  21.f)), module, DroneClone::SPREAD_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(       mm2px(Vec(113.5f,  35.f)), module, DroneClone::SPREAD_CV_INPUT));

		// MASS/TENSION/WEIGHT row (y=52mm) — satellites to right
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(21.98f, 52.f)), module, DroneClone::MASS_PARAM));
		addParam(createParamCentered<Trimpot>(       mm2px(Vec(31.91f,  44.f)), module, DroneClone::MASS_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(    mm2px(Vec(31.91f,  60.f)), module, DroneClone::MASS_CV_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(66.06f, 52.f)), module, DroneClone::TENSION_PARAM));
		addParam(createParamCentered<Trimpot>(       mm2px(Vec(76.82f,  44.f)), module, DroneClone::TENSION_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(    mm2px(Vec(76.82f,  60.f)), module, DroneClone::TENS_CV_INPUT));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(110.f, 52.f)), module, DroneClone::WEIGHT_PARAM));
		addParam(createParamCentered<Trimpot>(       mm2px(Vec(120.f,44.f)), module, DroneClone::WEIGHT_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(    mm2px(Vec(120.f,60.f)), module, DroneClone::WEIGHT_CV_INPUT));

		// SHIMMER/JAWARI/DRIFT row (y=66mm) — satellites to LEFT (avoids row-above conflict)
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(21.98f, 66.f)), module, DroneClone::SHIMMER_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(11.82f,  59.f)), module, DroneClone::SHIMMER_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(11.82f,  73.f)), module, DroneClone::SHIMMER_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(66.06f, 66.f)), module, DroneClone::JAWARI_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(54.36f,  59.f)), module, DroneClone::JAWARI_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(54.36f,  73.f)), module, DroneClone::JAWARI_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(110.f, 66.f)), module, DroneClone::DRIFT_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(98.09f,  59.f)), module, DroneClone::DRIFT_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(         mm2px(Vec(98.09f,  73.f)), module, DroneClone::DRIFT_CV_INPUT));

		// COLLAPSE row (y=85mm)
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(32.97f, 85.f)), module, DroneClone::DECAY_PARAM));
		addParam(createParamCentered<Trimpot>(          mm2px(Vec(47.27f,  78.f)), module, DroneClone::DECAY_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(       mm2px(Vec(47.27f,  92.f)), module, DroneClone::DECAY_CV_INPUT));

		addParam(createParamCentered<TL1105>(  mm2px(Vec(66.06f, 85.f)), module, DroneClone::CHOKE_PARAM));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(66.06f, 77.f)), module, DroneClone::CHOKE_LIGHT));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(99.04f, 85.f)), module, DroneClone::CHOKE_AMT_PARAM));
		addParam(createParamCentered<Trimpot>(          mm2px(Vec(113.5f,  78.f)), module, DroneClone::CHOKE_AMT_ATTEN_PARAM));
		addInput(createInputCentered<AFPortIn>(       mm2px(Vec(113.5f,  92.f)), module, DroneClone::CHOKE_AMT_CV_INPUT));

		// CV / IO row (y=101mm)
		addInput(createInputCentered<AFPortIn>( mm2px(Vec(16.55f,  101.f)), module, DroneClone::VOCT_INPUT));
		addOutput(createOutputCentered<AFPortOut>(mm2px(Vec(43.73f,  101.f)), module, DroneClone::VOCT_OUTPUT));
		addInput(createInputCentered<AFPortIn>( mm2px(Vec(115.2f, 101.f)), module, DroneClone::CHOKE_INPUT));

		// OUT / RTN row (y=116mm)
		addOutput(createOutputCentered<AFPortOut>(mm2px(Vec(88.64f, 116.f)), module, DroneClone::OUT_OUTPUT));
		addInput(createInputCentered<AFPortIn>( mm2px(Vec(112.3f, 116.f)), module, DroneClone::RTN_INPUT));
	}

	// SPREAD reads in cents, which does not tell you what you will hear:
	// the same detune beats at a different rate in every octave.
	void appendContextMenu(Menu* menu) override {
		DroneClone* m = dynamic_cast<DroneClone*>(module);
		if (!m)          // the module browser instantiates the widget with none
			return;
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel(
			"Beat rate: " + string::f("%.2f", m->beatRateHz_) + " Hz"));
	}
};

Model* modelDroneClone = createModel<DroneClone, DroneCloneWidget>("DroneClone");
