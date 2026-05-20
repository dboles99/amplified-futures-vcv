#include "plugin.hpp"

// ============================================================
// FEEDBACK GOVERNOR — controlled feedback send/return
//
// Design: Amplified Futures (steel finish, 12 HP)
//   AMOUNT: feedback level 0–0.95 (hard capped for safety)
//   TONE:   1-pole LP on the feedback path
//           0 = dark (100 Hz) → 1 = full bandwidth (20 kHz)
//   DECAY:  attenuates feedback: at 1 = −24 dB per pass
//           (AMOUNT × 2^(−4×DECAY)) — lets feedback die away
//   KILL:   button or gate input → instantly zeros feedback
//   SEND:   signal in from the main chain
//   RETURN: processed signal back into the chain
//
// Safety: DC blocker (5 Hz HP) + hard clip at ±10 V
//
// Typical patch: Wall Conductor OUT → Governor SEND
//                Governor RETURN → Send module IN B (B→A path)
//                or feed back into DroneClone feedback input
// ============================================================

struct FeedbackGovernor : Module {
	enum ParamId {
		AMOUNT_PARAM,
		AMOUNT_ATTEN_PARAM,
		TONE_PARAM,
		TONE_ATTEN_PARAM,
		DECAY_PARAM,
		DECAY_ATTEN_PARAM,
		KILL_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SEND_INPUT,
		AMOUNT_CV_INPUT,
		TONE_CV_INPUT,
		DECAY_CV_INPUT,
		KILL_INPUT,
		VOCT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		RETURN_OUTPUT,
		VOCT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float lpState  = 0.f;   // LP filter state
	float hpState  = 0.f;   // DC blocker state

	FeedbackGovernor() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(AMOUNT_PARAM,        0.f, 1.f, 0.5f, "Amount",  "%", 0.f, 100.f);
		configParam(AMOUNT_ATTEN_PARAM, -1.f, 1.f, 0.f,  "Amount attenuverter");
		configParam(TONE_PARAM,          0.f, 1.f, 0.8f, "Tone",    "%", 0.f, 100.f);
		configParam(TONE_ATTEN_PARAM,   -1.f, 1.f, 0.f,  "Tone attenuverter");
		configParam(DECAY_PARAM,         0.f, 1.f, 0.f,  "Decay",   "%", 0.f, 100.f);
		configParam(DECAY_ATTEN_PARAM,  -1.f, 1.f, 0.f,  "Decay attenuverter");
		configButton(KILL_PARAM, "Kill feedback");

		getParamQuantity(AMOUNT_PARAM)->description = "Feedback level. Capped at 0.95 to prevent infinite runaway.";
		getParamQuantity(TONE_PARAM)->description   = "Low-pass on feedback path. 0=100Hz (dark) 1=20kHz (full)";
		getParamQuantity(DECAY_PARAM)->description  = "Attenuates feedback per pass: 0=sustain 1=−24dB (fast fade)";

		configInput(SEND_INPUT,       "Signal in (from chain)");
		configInput(AMOUNT_CV_INPUT,  "Amount CV");
		configInput(TONE_CV_INPUT,    "Tone CV");
		configInput(DECAY_CV_INPUT,   "Decay CV");
		configInput(KILL_INPUT,       "Kill gate");
		configInput(VOCT_INPUT,       "V/oct (thru)");
		configOutput(RETURN_OUTPUT,   "Signal out (back into chain)");
		configOutput(VOCT_OUTPUT,     "V/oct (thru)");
	}

	float modp(int param, int atten, int cv, float lo, float hi) {
		float v = params[param].getValue();
		if (inputs[cv].isConnected())
			v += params[atten].getValue() * inputs[cv].getVoltage() / 5.f;
		return clamp(v, lo, hi);
	}

	void process(const ProcessArgs& args) override {
		// Kill: button or gate instantly zeros feedback state
		bool killed = (params[KILL_PARAM].getValue() > 0.5f)
		           || (inputs[KILL_INPUT].getVoltage() >= 1.f);
		if (killed) {
			lpState = 0.f;
			hpState = 0.f;
			outputs[RETURN_OUTPUT].setVoltage(0.f);
			outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage());
			return;
		}

		float amount = modp(AMOUNT_PARAM, AMOUNT_ATTEN_PARAM, AMOUNT_CV_INPUT, 0.f, 0.95f);
		float tone   = modp(TONE_PARAM,   TONE_ATTEN_PARAM,   TONE_CV_INPUT,   0.f, 1.f);
		float decay  = modp(DECAY_PARAM,  DECAY_ATTEN_PARAM,  DECAY_CV_INPUT,  0.f, 1.f);

		float x = inputs[SEND_INPUT].getVoltage();

		// 1-pole LP filter: tone=0 → 100 Hz, tone=1 → 20 kHz (exponential)
		float cutoff  = 100.f * std::pow(200.f, tone);
		float lpAlpha = 1.f - std::exp(-2.f * float(M_PI) * cutoff * args.sampleTime);
		lpState += lpAlpha * (x - lpState);

		// DC blocker: fixed 5 Hz HP prevents feedback from accumulating DC
		float hpAlpha = 1.f - std::exp(-2.f * float(M_PI) * 5.f * args.sampleTime);
		hpState += hpAlpha * (lpState - hpState);
		float filtered = lpState - hpState;

		// Decay: exponential additional attenuation per pass
		// AMOUNT × 2^(−4 × DECAY) → ranges from AMOUNT×1.0 to AMOUNT×0.0625
		float effAmount = amount * std::pow(0.5f, decay * 4.f);

		float returned = clamp(filtered * effAmount, -10.f, 10.f);

		outputs[RETURN_OUTPUT].setVoltage(returned);
		outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage());
	}
};

// ============================================================
// WIDGET  (12 HP)
// ============================================================

struct FeedbackGovernorWidget : ModuleWidget {
	FeedbackGovernorWidget(FeedbackGovernor* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/FeedbackGovernor.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// ── Row 1: AMOUNT (L) | TONE (R) ──────────────────────────
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 38.f)), module, FeedbackGovernor::AMOUNT_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec( 9.24f, 50.f)), module, FeedbackGovernor::AMOUNT_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(21.24f, 50.f)), module, FeedbackGovernor::AMOUNT_CV_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(45.72f, 38.f)), module, FeedbackGovernor::TONE_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec(39.72f, 50.f)), module, FeedbackGovernor::TONE_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(51.72f, 50.f)), module, FeedbackGovernor::TONE_CV_INPUT));

		// ── Row 2: DECAY (L) | KILL button + gate (R) ────────────
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24f, 70.f)), module, FeedbackGovernor::DECAY_PARAM));
		addParam(createParamCentered<Trimpot>(            mm2px(Vec( 9.24f, 82.f)), module, FeedbackGovernor::DECAY_ATTEN_PARAM));
		addInput(createInputCentered<PJ301MPort>(         mm2px(Vec(21.24f, 82.f)), module, FeedbackGovernor::DECAY_CV_INPUT));

		addParam(createParamCentered<TL1105>(    mm2px(Vec(45.72f, 68.f)), module, FeedbackGovernor::KILL_PARAM));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.72f, 82.f)), module, FeedbackGovernor::KILL_INPUT));

		// ── IO ─────────────────────────────────────────────────────
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(15.24f, 104.f)), module, FeedbackGovernor::SEND_INPUT));
		addInput(createInputCentered<PJ301MPort>( mm2px(Vec(30.48f, 104.f)), module, FeedbackGovernor::VOCT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24f, 116.f)), module, FeedbackGovernor::RETURN_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.48f, 116.f)), module, FeedbackGovernor::VOCT_OUTPUT));
	}

	void draw(const DrawArgs& args) override {
		ModuleWidget::draw(args);
		if (!APP->window->uiFont) return;
		nvgFontFaceId(args.vg, APP->window->uiFont->handle);
		nvgFontSize(args.vg, 11.f);
		nvgTextLetterSpacing(args.vg, 2.f);
		nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
		nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(args.vg, box.size.x / 2.f, 11.f, "GOVERNOR", NULL);
	}
};

Model* modelFeedbackGovernor = createModel<FeedbackGovernor, FeedbackGovernorWidget>("FeedbackGovernor");
