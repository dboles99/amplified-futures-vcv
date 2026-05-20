#include "plugin.hpp"

// ============================================================
// STRING MASS CORE — 16-voice harmonic mass oscillator
//
// Design: Amplified Futures (steel finish, 16 HP)
//   MASS:    1–16 internal voices  (snap, CV-able)
//   SPREAD:  per-voice detune 0–50 cents
//   TIMBRE:  sine → third-bridge harmonic stack
//   MODE:    UNIS / HARM / JUST / MICRO (snap)
//   SECTION: 1 / 2 / 4 harmonic sections (snap, HARM mode only)
//
// Modes:
//   UNIS  — all M voices at fundamental, symmetric ±SPREAD cents
//   HARM  — M voices divided into SECTION groups; each group at
//            an odd harmonic (1,3,5,7→octave-reduced), within-
//            group spread creates dense beating layers
//   JUST  — M voices across JI chromatic ratios + narrow SPREAD
//   MICRO — all voices at fundamental with per-voice slow vibrato
//            at different rates; produces organic shimmer
//
// Amplitude: 1/√M normalisation → ~5V output; tanh soft-clip
// Polyphonic: one mass per V/OCT channel, output poly matches
// ============================================================

// Octave-reduced odd harmonics 1,3,5,7,9,11,13,15
static const float harmRatios[8] = {
    1.f,       // 1st  — fundamental
    1.5f,      // 3rd  — perfect fifth
    1.25f,     // 5th  — major third
    1.75f,     // 7th  — harmonic 7th (blue note)
    1.125f,    // 9th  — major second
    1.375f,    // 11th — tritone
    1.625f,    // 13th — minor sixth (sharp)
    1.875f     // 15th — major seventh
};

// JI chromatic ratios (12-note, Ptolemaic)
static const float justRatios[12] = {
    1.f,        // 1/1   unison
    1.0667f,    // 16/15 minor second
    1.125f,     // 9/8   major second
    1.2f,       // 6/5   minor third
    1.25f,      // 5/4   major third
    1.3333f,    // 4/3   perfect fourth
    1.4063f,    // 45/32 tritone
    1.5f,       // 3/2   perfect fifth
    1.6f,       // 8/5   minor sixth
    1.6667f,    // 5/3   major sixth
    1.7778f,    // 16/9  minor seventh
    1.875f      // 15/8  major seventh
};

struct StringMassCore : Module {
	enum ParamId {
		MASS_PARAM,
		MASS_ATTEN_PARAM,
		SPREAD_PARAM,
		SPREAD_ATTEN_PARAM,
		TIMBRE_PARAM,
		TIMBRE_ATTEN_PARAM,
		MODE_PARAM,
		SECTION_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		VOCT_INPUT,
		MASS_CV_INPUT,
		SPREAD_CV_INPUT,
		TIMBRE_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		AUDIO_OUTPUT,
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	// [voice][poly_channel]
	float phase[16][16]      = {};
	float microPhase[16][16] = {};  // slow LFO phase for MICRO mode

	StringMassCore() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(MASS_PARAM,           1.f,  16.f, 4.f,  "Mass",    " voices");
		configParam(MASS_ATTEN_PARAM,    -1.f,   1.f, 0.f,  "Mass attenuverter");
		configParam(SPREAD_PARAM,         0.f,   1.f, 0.3f, "Spread",  "%", 0.f, 100.f);
		configParam(SPREAD_ATTEN_PARAM,  -1.f,   1.f, 0.f,  "Spread attenuverter");
		configParam(TIMBRE_PARAM,         0.f,   1.f, 0.3f, "Timbre",  "%", 0.f, 100.f);
		configParam(TIMBRE_ATTEN_PARAM,  -1.f,   1.f, 0.f,  "Timbre attenuverter");
		configParam(MODE_PARAM,           0.f,   3.f, 1.f,  "Mode");
		configParam(SECTION_PARAM,        0.f,   2.f, 1.f,  "Sections");

		getParamQuantity(MASS_PARAM)->snapEnabled    = true;
		getParamQuantity(MODE_PARAM)->snapEnabled    = true;
		getParamQuantity(SECTION_PARAM)->snapEnabled = true;
		getParamQuantity(MODE_PARAM)->description    = "0=UNIS  1=HARM  2=JUST  3=MICRO";
		getParamQuantity(SECTION_PARAM)->description = "0=1 section  1=2 sections  2=4 sections (HARM mode)";

		configInput(VOCT_INPUT,      "V/oct pitch (poly)");
		configInput(MASS_CV_INPUT,   "Mass CV");
		configInput(SPREAD_CV_INPUT, "Spread CV");
		configInput(TIMBRE_CV_INPUT, "Timbre CV");
		configOutput(AUDIO_OUTPUT,   "Audio mass (poly)");
		configOutput(VOCT_OUTPUT,    "V/oct thru (poly)");

		// Stagger initial phases to avoid startup coherence spike
		for (int v = 0; v < 16; v++)
			for (int c = 0; c < 16; c++) {
				phase[v][c]      = float(v) / 16.f;
				microPhase[v][c] = float(v) / 16.f;
			}
	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float val = params[param].getValue();
		if (inputs[cv].isConnected())
			val += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(val, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		int polyCh = std::max(1, inputs[VOCT_INPUT].getChannels());
		outputs[AUDIO_OUTPUT].setChannels(polyCh);
		outputs[VOCT_OUTPUT].setChannels(polyCh);

		int M = (int)std::round(modp(MASS_PARAM, MASS_ATTEN_PARAM, MASS_CV_INPUT, 1.f, 16.f));
		float spread = modp(SPREAD_PARAM, SPREAD_ATTEN_PARAM, SPREAD_CV_INPUT, 0.f, 1.f);
		float timbre = modp(TIMBRE_PARAM, TIMBRE_ATTEN_PARAM, TIMBRE_CV_INPUT, 0.f, 1.f);

		int mode     = clamp((int)std::round(params[MODE_PARAM].getValue()),    0, 3);
		int secParam = clamp((int)std::round(params[SECTION_PARAM].getValue()), 0, 2);
		int sections = (secParam == 0) ? 1 : (secParam == 1) ? 2 : 4;
		sections = std::min(sections, M);

		float spreadCents = spread * 50.f;   // 0–50 cents total spread
		float timbreNorm  = 1.f + timbre * 1.08f;
		float norm        = 1.f / std::sqrt(float(M));

		for (int c = 0; c < polyCh; c++) {
			float voct = inputs[VOCT_INPUT].getVoltage(c);
			float freq = dsp::FREQ_C4 * std::pow(2.f, voct);

			float sum = 0.f;

			for (int v = 0; v < M; v++) {
				float voiceFreq;
				// Linear spread position across all voices: -1 to +1
				float spreadPos = (M > 1) ? (2.f * v / float(M - 1) - 1.f) : 0.f;

				if (mode == 0) {
					// ── UNIS: symmetric spread around fundamental ────────
					float cents = spreadPos * spreadCents;
					voiceFreq   = freq * std::pow(2.f, cents / 1200.f);

				} else if (mode == 1) {
					// ── HARM: voices in harmonic series sections ─────────
					int sec      = clamp((v * sections) / M, 0, 7);
					int secStart = sec * M / sections;
					int secEnd   = (sec + 1) * M / sections;
					int secLen   = std::max(1, secEnd - secStart);
					int posInSec = v - secStart;
					float secPos = (secLen > 1) ? (2.f * posInSec / float(secLen - 1) - 1.f) : 0.f;

					float cents = secPos * spreadCents;
					voiceFreq   = freq * harmRatios[sec] * std::pow(2.f, cents / 1200.f);

				} else if (mode == 2) {
					// ── JUST: M voices mapped to JI chromatic ratios ─────
					int idx    = clamp((v * 12) / M, 0, 11);
					float cents = spreadPos * spreadCents * 0.3f;  // narrower spread in JUST
					voiceFreq  = freq * justRatios[idx] * std::pow(2.f, cents / 1200.f);

				} else {
					// ── MICRO: slow per-voice vibrato at different rates ──
					// Each voice has its own LFO rate (0.03–0.26 Hz), creating
					// independent beating that accumulates psychoacoustically
					float lfoRate = 0.03f + float(v) * 0.015f;
					microPhase[v][c] += lfoRate * args.sampleTime;
					if (microPhase[v][c] >= 1.f) microPhase[v][c] -= 1.f;
					float cents = std::sin(2.f * float(M_PI) * microPhase[v][c]) * spreadCents;
					voiceFreq   = freq * std::pow(2.f, cents / 1200.f);
				}

				// Advance phasor
				phase[v][c] += voiceFreq * args.sampleTime;
				if (phase[v][c] >= 1.f) phase[v][c] -= 1.f;

				// Waveform: sine + harmonics 2–4 (third-bridge harmonic stack)
				float p = phase[v][c];
				float s = (std::sin(2.f * float(M_PI) * p)
				         + timbre * 0.5f  * std::sin(4.f * float(M_PI) * p)
				         + timbre * 0.33f * std::sin(6.f * float(M_PI) * p)
				         + timbre * 0.25f * std::sin(8.f * float(M_PI) * p)) / timbreNorm;

				sum += s;
			}

			// 1/√M normalisation + tanh soft ceiling
			outputs[AUDIO_OUTPUT].setVoltage(5.f * std::tanh(sum * norm), c);
			outputs[VOCT_OUTPUT].setVoltage(voct, c);
		}
	}
};

// ============================================================
// WIDGET  (16 HP)
// ============================================================

struct StringMassCoreWidget : ModuleWidget {
	StringMassCoreWidget(StringMassCore* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/StringMassCore.svg")));

		// 16HP screws
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// ── Row 1: MASS (L) | SPREAD (R) ──────────────────────────
		// L=15mm, satellite at L+8=23mm
		// R=62mm, satellite at R+8=70mm

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 38.f)), module, StringMassCore::MASS_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(23.f, 31.f)), module, StringMassCore::MASS_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(23.f, 45.f)), module, StringMassCore::MASS_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(62.f, 38.f)), module, StringMassCore::SPREAD_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(70.f, 31.f)), module, StringMassCore::SPREAD_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(70.f, 45.f)), module, StringMassCore::SPREAD_CV_INPUT));

		// ── Row 2: TIMBRE (L) | MODE (R, discrete) ────────────────

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.f, 68.f)), module, StringMassCore::TIMBRE_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(23.f, 61.f)), module, StringMassCore::TIMBRE_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(23.f, 75.f)), module, StringMassCore::TIMBRE_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(62.f, 68.f)), module, StringMassCore::MODE_PARAM));

		// ── Row 3: SECTION (centre, discrete) ─────────────────────

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(40.f, 92.f)), module, StringMassCore::SECTION_PARAM));

		// ── Row 4: IO ──────────────────────────────────────────────
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(15.f, 110.f)), module, StringMassCore::VOCT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(40.f, 110.f)), module, StringMassCore::VOCT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(65.f, 110.f)), module, StringMassCore::AUDIO_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 10.f);
		nvgTextLetterSpacing(args.vg, 2.f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "STRING MASS", NULL);
	}
};

Model* modelStringMassCore = createModel<StringMassCore, StringMassCoreWidget>("StringMassCore");
