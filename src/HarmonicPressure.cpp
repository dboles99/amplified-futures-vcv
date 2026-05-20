#include "plugin.hpp"

// ============================================================
// HARMONIC PRESSURE — harmonic series pitch CV generator
//
// Design: Amplified Futures (steel finish, 14 HP)
//   PITCH:   root pitch offset (-2 to +2 oct)  [CV + atten]
//   SPREAD:  per-partial ensemble detuning      [CV + atten]
//   PARTIAL: first partial index (1–16)         [snap]
//   COUNT:   number of partials to output (1–16)[snap]
//   TUNING:  JUST / EQUAL / MICRO               [snap 3-pos]
//
// Outputs polyphonic V/OCT — COUNT channels, each a harmonic
// partial of the root:  partial n  →  root + log2(n) octaves
//
// JUST:  exact harmonic series ratios (pure JI)
// EQUAL: each partial rounded to nearest 12-TET semitone
// MICRO: JI + deterministic per-partial intonation offset
//        (SPREAD controls magnitude — simulates ensemble tuning
//        variation the way 100 massed voices each tune
//        the same pitch slightly differently)
// ============================================================

struct HarmonicPressure : Module {
	enum ParamId {
		PITCH_PARAM,
		PITCH_ATTEN_PARAM,
		SPREAD_PARAM,
		SPREAD_ATTEN_PARAM,
		PARTIAL_PARAM,
		COUNT_PARAM,
		TUNING_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		PITCH_CV_INPUT,
		SPREAD_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	HarmonicPressure() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PITCH_PARAM,        -2.f,  2.f,  0.f,  "Pitch",   " Oct");
		configParam(PITCH_ATTEN_PARAM,  -1.f,  1.f,  0.f,  "Pitch attenuverter");
		configParam(SPREAD_PARAM,        0.f,  1.f,  0.f,  "Spread",  "%", 0.f, 100.f);
		configParam(SPREAD_ATTEN_PARAM, -1.f,  1.f,  0.f,  "Spread attenuverter");
		configParam(PARTIAL_PARAM,       1.f,  16.f, 1.f,  "First partial");
		configParam(COUNT_PARAM,         1.f,  16.f, 8.f,  "Partial count");
		configParam(TUNING_PARAM,        0.f,  2.f,  0.f,  "Tuning mode");

		getParamQuantity(PARTIAL_PARAM)->snapEnabled = true;
		getParamQuantity(COUNT_PARAM)->snapEnabled   = true;
		getParamQuantity(TUNING_PARAM)->snapEnabled  = true;

		getParamQuantity(PARTIAL_PARAM)->description = "Starting harmonic partial (1 = fundamental)";
		getParamQuantity(COUNT_PARAM)->description   = "Number of partials to output as poly channels";
		getParamQuantity(TUNING_PARAM)->description  = "0=JUST (pure JI)  1=EQUAL (12-TET)  2=MICRO (JI + ensemble spread)";

		configInput(VOCT_INPUT,      "Root V/oct");
		configInput(PITCH_CV_INPUT,  "Pitch offset CV");
		configInput(SPREAD_CV_INPUT, "Spread CV");
		configOutput(VOCT_OUTPUT,    "Harmonic series V/oct (poly)");
	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		// Root pitch: VOCT input + PITCH knob offset
		float root = inputs[VOCT_INPUT].getVoltage();   // 0V if disconnected
		float pitchOffset = params[PITCH_PARAM].getValue();
		if (inputs[PITCH_CV_INPUT].isConnected())
			pitchOffset += params[PITCH_ATTEN_PARAM].getValue()
			             * inputs[PITCH_CV_INPUT].getVoltage() / 5.f;
		root += clamp(pitchOffset, -4.f, 4.f);

		float spread = modp(SPREAD_PARAM, SPREAD_ATTEN_PARAM, SPREAD_CV_INPUT, 0.f, 1.f);
		float spreadCents = spread * 20.f;   // max ±20 cents per partial

		int first = clamp((int)std::round(params[PARTIAL_PARAM].getValue()), 1, 16);
		int count = clamp((int)std::round(params[COUNT_PARAM].getValue()),   1, 16);
		// Clamp so we don't exceed partial 32 (keeps V/OCT in sane range)
		count = std::min(count, 33 - first);

		int tuning = clamp((int)std::round(params[TUNING_PARAM].getValue()), 0, 2);

		outputs[VOCT_OUTPUT].setChannels(count);

		for (int i = 0; i < count; i++) {
			int n = first + i;   // partial index (1 = fundamental)

			// Exact harmonic series: partial n is at log2(n) octaves above root
			float voct = root + std::log2(float(n));

			if (tuning == 1) {
				// EQUAL: quantise to nearest 12-TET semitone
				voct = std::round(voct * 12.f) / 12.f;

			} else if (tuning == 2) {
				// MICRO: JI + per-partial deterministic offset
				// sin(n × golden_angle) gives a quasi-random but fixed offset
				// per partial — same every time, like a musician with a
				// characteristic sharp or flat tendency on that note
				float microOct = std::sin(float(n) * 2.39996f) * spreadCents / 1200.f;
				voct += microOct;
			}

			// JUST and EQUAL modes: SPREAD adds a consistent per-partial
			// offset that scales with SPREAD (0 = pure, >0 = ensemble colour)
			if (tuning != 2) {
				float colour = std::sin(float(n) * 1.61803f) * spreadCents / 1200.f;
				voct += colour;
			}

			outputs[VOCT_OUTPUT].setVoltage(voct, i);
		}
	}
};

// ============================================================
// WIDGET  (14 HP)
// ============================================================

struct HarmonicPressureWidget : ModuleWidget {
	HarmonicPressureWidget(HarmonicPressure* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/HarmonicPressure.svg")));

		// 14HP screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// ── Row 1: PITCH (L) | SPREAD (R) ─────────────────────────
		// L=15mm  sat=23mm (+8)   R=55mm  sat=63mm (+8)

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 40.f)), module, HarmonicPressure::PITCH_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(23.f, 33.f)), module, HarmonicPressure::PITCH_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(23.f, 47.f)), module, HarmonicPressure::PITCH_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.f, 40.f)), module, HarmonicPressure::SPREAD_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(63.f, 33.f)), module, HarmonicPressure::SPREAD_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(63.f, 47.f)), module, HarmonicPressure::SPREAD_CV_INPUT));

		// ── Row 2: PARTIAL (L) | COUNT (R) — discrete snap ────────

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 72.f)), module, HarmonicPressure::PARTIAL_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.f, 72.f)), module, HarmonicPressure::COUNT_PARAM));

		// ── Row 3: TUNING (centre) — discrete snap ────────────────

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(35.f, 96.f)), module, HarmonicPressure::TUNING_PARAM));

		// ── Row 4: IO ──────────────────────────────────────────────
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(15.f, 114.f)), module, HarmonicPressure::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.f, 114.f)), module, HarmonicPressure::VOCT_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 9.5f);
		nvgTextLetterSpacing(args.vg, 1.5f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "HARM PRESS", NULL);
	}
};

Model* modelHarmonicPressure = createModel<HarmonicPressure, HarmonicPressureWidget>("HarmonicPressure");
