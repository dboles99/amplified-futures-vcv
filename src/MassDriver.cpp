#include "plugin.hpp"

// ============================================================
// MASS DRIVER — AF-01, 32HP
//
// 16-channel no-wave signal mixer / mass conductor.
// Left spine:  CH 1–8  (GAIN knob · MUTE button · IN jack)
// Right spine: CH 9–16 (IN jack · MUTE button · GAIN knob)
// Centre spine: master DENSITY · PRESSURE · WIDTH · FEEDBACK
//               MASS (output level) · COLLAPSE gate
//
// Signal flow:
//   per-channel: sig × GAIN × densityGain × (1-muted)
//   pan: linear spread -WIDTH to +WIDTH across 16 channels
//   mixL/R += sig * cos/sin(panR * π/2)
//   mix   += feedbackL/R * FEEDBACK
//   AUX L/R = pre-PRESSURE stereo (with MASS)
//   SUM     = mono sum (with MASS)
//   OUT L/R = tanh(mix * drive) * collapseEnv * MASS
//
// DENSITY: sweep 0→16 channels in
// PRESSURE: drive 1–4× into tanh
// WIDTH: stereo spread 0 = mono, 1 = full L/R
// MASS: master output level 0–1 (×5V)
// FEEDBACK: 1-sample feedback, capped 0–0.92
// ============================================================

struct MassDriver : Module {
    enum ParamId {
        ENUMS(GAIN_PARAM, 16),        // 0–15:  per-channel gain (0–2×)
        ENUMS(MUTE_PARAM, 16),        // 16–31: per-channel mute buttons
        DENSITY_PARAM,                // 32
        DENSITY_ATTEN_PARAM,          // 33
        PRESSURE_PARAM,               // 34
        PRESSURE_ATTEN_PARAM,         // 35
        WIDTH_PARAM,                  // 36
        WIDTH_ATTEN_PARAM,            // 37
        MASS_PARAM,                   // 38
        MASS_ATTEN_PARAM,             // 39
        FEEDBACK_PARAM,               // 40
        FEEDBACK_ATTEN_PARAM,         // 41
        COLLAPSE_PARAM,               // 42
        PARAMS_LEN
    };
    enum InputId {
        ENUMS(CH_INPUT, 16),          // 0–15:  per-channel audio
        DENSITY_CV_INPUT,             // 16
        PRESSURE_CV_INPUT,            // 17
        WIDTH_CV_INPUT,               // 18
        MASS_CV_INPUT,                // 19
        FEEDBACK_CV_INPUT,            // 20
        COLLAPSE_INPUT,               // 21
        VOCT_INPUT,                   // 22
        INPUTS_LEN
    };
    enum OutputId {
        OUT_L_OUTPUT,                 // 0
        OUT_R_OUTPUT,                 // 1
        AUX_L_OUTPUT,                 // 2
        AUX_R_OUTPUT,                 // 3
        SUM_OUTPUT,                   // 4
        VOCT_OUTPUT,                  // 5
        OUTPUTS_LEN
    };
    enum LightId {
        ENUMS(MUTE_LIGHT, 16),        // 0–15: red when muted
        LIGHTS_LEN
    };

    bool   muted[16]    = {};
    float  collapseEnv  = 1.f;
    float  feedbackL    = 0.f;
    float  feedbackR    = 0.f;
    dsp::BooleanTrigger muteTrig[16];

    float modp(int p, int a, int c, float lo, float hi) {
        float base  = params[p].getValue();
        float atten = params[a].getValue();
        float cv    = inputs[c].getVoltage() / 10.f;
        return clamp(base + atten * cv, 0.f, 1.f) * (hi - lo) + lo;
    }

    MassDriver() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        for (int i = 0; i < 16; i++) {
            configParam(GAIN_PARAM + i, 0.f, 2.f, 0.75f, string::f("CH%d gain", i + 1), "×");
            configButton(MUTE_PARAM + i, string::f("CH%d mute", i + 1));
            configInput(CH_INPUT + i, string::f("CH%d audio", i + 1));
            configLight(MUTE_LIGHT + i, string::f("CH%d mute", i + 1));
        }

        configParam(DENSITY_PARAM,        0.f,  1.f,  1.f,   "Density",  "%", 0.f, 100.f);
        configParam(DENSITY_ATTEN_PARAM, -1.f,  1.f,  0.f,   "Density attenuverter");
        configParam(PRESSURE_PARAM,       0.f,  1.f,  0.25f, "Pressure", "%", 0.f, 100.f);
        configParam(PRESSURE_ATTEN_PARAM,-1.f,  1.f,  0.f,   "Pressure attenuverter");
        configParam(WIDTH_PARAM,          0.f,  1.f,  0.8f,  "Width",    "%", 0.f, 100.f);
        configParam(WIDTH_ATTEN_PARAM,   -1.f,  1.f,  0.f,   "Width attenuverter");
        configParam(MASS_PARAM,           0.f,  1.f,  0.75f, "Mass",     "%", 0.f, 100.f);
        configParam(MASS_ATTEN_PARAM,    -1.f,  1.f,  0.f,   "Mass attenuverter");
        configParam(FEEDBACK_PARAM,       0.f,  1.f,  0.f,   "Feedback", "%", 0.f, 100.f);
        configParam(FEEDBACK_ATTEN_PARAM,-1.f,  1.f,  0.f,   "Feedback attenuverter");
        configButton(COLLAPSE_PARAM, "Collapse");

        configInput(DENSITY_CV_INPUT,  "Density CV");
        configInput(PRESSURE_CV_INPUT, "Pressure CV");
        configInput(WIDTH_CV_INPUT,    "Width CV");
        configInput(MASS_CV_INPUT,     "Mass CV");
        configInput(FEEDBACK_CV_INPUT, "Feedback CV");
        configInput(COLLAPSE_INPUT,    "Collapse gate");
        configInput(VOCT_INPUT,        "V/oct");

        configOutput(OUT_L_OUTPUT,  "Out L");
        configOutput(OUT_R_OUTPUT,  "Out R");
        configOutput(AUX_L_OUTPUT,  "Aux L (pre-pressure)");
        configOutput(AUX_R_OUTPUT,  "Aux R (pre-pressure)");
        configOutput(SUM_OUTPUT,    "Sum (mono)");
        configOutput(VOCT_OUTPUT,   "V/oct thru");
    }

    void process(const ProcessArgs& args) override {
        // ── Mute toggles ─────────────────────────────────────────────
        for (int i = 0; i < 16; i++) {
            if (muteTrig[i].process(params[MUTE_PARAM + i].getValue() > 0.5f))
                muted[i] = !muted[i];
            lights[MUTE_LIGHT + i].setBrightness(muted[i] ? 1.f : 0.f);
        }

        // ── Master params ─────────────────────────────────────────────
        float density  = modp(DENSITY_PARAM,  DENSITY_ATTEN_PARAM,  DENSITY_CV_INPUT,  0.f, 1.f);
        float pressure = modp(PRESSURE_PARAM, PRESSURE_ATTEN_PARAM, PRESSURE_CV_INPUT, 0.f, 1.f);
        float width    = modp(WIDTH_PARAM,    WIDTH_ATTEN_PARAM,    WIDTH_CV_INPUT,    0.f, 1.f);
        float mass     = modp(MASS_PARAM,     MASS_ATTEN_PARAM,     MASS_CV_INPUT,     0.f, 1.f);
        float feedback = modp(FEEDBACK_PARAM, FEEDBACK_ATTEN_PARAM, FEEDBACK_CV_INPUT, 0.f, 0.92f);

        // ── Collapse envelope ─────────────────────────────────────────
        bool collapseGate = params[COLLAPSE_PARAM].getValue() > 0.5f
                         || inputs[COLLAPSE_INPUT].getVoltage() > 1.f;
        if (collapseGate) {
            collapseEnv = 0.f;
        } else {
            float recoveryCoeff = 1.f - std::exp(-args.sampleTime / 1.5f);  // ~1.5s recovery
            collapseEnv += (1.f - collapseEnv) * recoveryCoeff;
        }

        // ── Mix ───────────────────────────────────────────────────────
        float mixL = feedbackL * feedback;
        float mixR = feedbackR * feedback;
        float auxL = 0.f;
        float auxR = 0.f;
        float sum  = 0.f;

        for (int i = 0; i < 16; i++) {
            if (muted[i] || !inputs[CH_INPUT + i].isConnected())
                continue;

            float densityGain = clamp(density * 16.f - float(i), 0.f, 1.f);
            float chGain = params[GAIN_PARAM + i].getValue();
            float sig = inputs[CH_INPUT + i].getVoltageSum() * chGain * densityGain;

            // Linear pan spread: ch 0 at -width, ch 15 at +width
            float panPos = (2.f * float(i) / 15.f - 1.f) * width;
            float panR   = (panPos + 1.f) * 0.5f;
            float L = sig * std::cos(panR * float(M_PI) * 0.5f);
            float R = sig * std::sin(panR * float(M_PI) * 0.5f);

            mixL += L;
            mixR += R;
            auxL += L;
            auxR += R;
            sum  += sig;
        }

        // ── Outputs ───────────────────────────────────────────────────
        float scale = mass * 5.f;

        // AUX: pre-pressure (dry stereo)
        outputs[AUX_L_OUTPUT].setVoltage(auxL / 5.f * scale);
        outputs[AUX_R_OUTPUT].setVoltage(auxR / 5.f * scale);
        outputs[SUM_OUTPUT].setVoltage(sum  / 5.f * scale);

        // OUT: tanh saturation + collapse
        float drive = 1.f + pressure * 3.f;
        float outL  = 5.f * std::tanh(mixL * drive / 5.f) * collapseEnv * mass;
        float outR  = 5.f * std::tanh(mixR * drive / 5.f) * collapseEnv * mass;

        outputs[OUT_L_OUTPUT].setVoltage(outL);
        outputs[OUT_R_OUTPUT].setVoltage(outR);

        feedbackL = outL;
        feedbackR = outR;

        // V/OCT thru
        int voctCh = inputs[VOCT_INPUT].getChannels();
        outputs[VOCT_OUTPUT].setChannels(voctCh);
        for (int c = 0; c < voctCh; c++)
            outputs[VOCT_OUTPUT].setVoltage(inputs[VOCT_INPUT].getVoltage(c), c);
    }

    json_t* dataToJson() override {
        json_t* root = json_object();
        json_t* mutesJ = json_array();
        for (int i = 0; i < 16; i++)
            json_array_append_new(mutesJ, json_boolean(muted[i]));
        json_object_set_new(root, "muted", mutesJ);
        return root;
    }

    void dataFromJson(json_t* root) override {
        json_t* mutesJ = json_object_get(root, "muted");
        if (mutesJ) {
            for (int i = 0; i < 16; i++) {
                json_t* v = json_array_get(mutesJ, i);
                if (v) muted[i] = json_boolean_value(v);
            }
        }
    }
};


struct MassDriverWidget : ModuleWidget {
    MassDriverWidget(MassDriver* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/MassDriver.svg")));

        // Corner screws
        addChild(createWidget<ScrewBlack>(mm2px(Vec(7.5f,   1.f))));
        addChild(createWidget<ScrewBlack>(mm2px(Vec(152.5f, 1.f))));
        addChild(createWidget<ScrewBlack>(mm2px(Vec(7.5f,   124.f))));
        addChild(createWidget<ScrewBlack>(mm2px(Vec(152.5f, 124.f))));

        // ── Left channel strips (CH 1–8) ─────────────────────────────
        // SVG px coords → mm: divide by 3
        // GAIN knob x=52px=17.33mm, MUTE x=86px=28.67mm, IN x=104px=34.67mm
        // Y rows: 44, 84, 124, 164, 204, 244, 284, 324 px → /3 mm
        static const float leftY[8] = { 44, 84, 124, 164, 204, 244, 284, 324 };
        for (int i = 0; i < 8; i++) {
            float y = leftY[i] / 3.f;
            addParam(createParamCentered<RoundBlackKnob>(
                mm2px(Vec(17.33f, y)), module, MassDriver::GAIN_PARAM + i));
            addParam(createParamCentered<TL1105>(
                mm2px(Vec(28.67f, y)), module, MassDriver::MUTE_PARAM + i));
            addChild(createLightCentered<SmallLight<RedLight>>(
                mm2px(Vec(28.67f, y - 4.f)), module, MassDriver::MUTE_LIGHT + i));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(34.67f, y)), module, MassDriver::CH_INPUT + i));
        }

        // ── Right channel strips (CH 9–16) ───────────────────────────
        // IN x=376px=125.33mm, MUTE x=394px=131.33mm, GAIN x=428px=142.67mm
        static const float rightY[8] = { 44, 84, 124, 164, 204, 244, 284, 324 };
        for (int i = 0; i < 8; i++) {
            float y = rightY[i] / 3.f;
            int ch = i + 8;
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(125.33f, y)), module, MassDriver::CH_INPUT + ch));
            addParam(createParamCentered<TL1105>(
                mm2px(Vec(131.33f, y)), module, MassDriver::MUTE_PARAM + ch));
            addChild(createLightCentered<SmallLight<RedLight>>(
                mm2px(Vec(131.33f, y - 4.f)), module, MassDriver::MUTE_LIGHT + ch));
            addParam(createParamCentered<RoundBlackKnob>(
                mm2px(Vec(142.67f, y)), module, MassDriver::GAIN_PARAM + ch));
        }

        // ── Master spine ─────────────────────────────────────────────
        // DENSITY: knob (70, 24) atten (64.1, 33.33) cv (75.9, 33.33)
        addParam(createParamCentered<RoundLargeBlackKnob>(
            mm2px(Vec(70.f, 24.f)), module, MassDriver::DENSITY_PARAM));
        addParam(createParamCentered<Trimpot>(
            mm2px(Vec(64.1f, 33.33f)), module, MassDriver::DENSITY_ATTEN_PARAM));
        addInput(createInputCentered<PJ301MPort>(
            mm2px(Vec(75.9f, 33.33f)), module, MassDriver::DENSITY_CV_INPUT));

        // PRESSURE: knob (90, 24) atten (84.1, 33.33) cv (95.9, 33.33)
        addParam(createParamCentered<RoundLargeBlackKnob>(
            mm2px(Vec(90.f, 24.f)), module, MassDriver::PRESSURE_PARAM));
        addParam(createParamCentered<Trimpot>(
            mm2px(Vec(84.1f, 33.33f)), module, MassDriver::PRESSURE_ATTEN_PARAM));
        addInput(createInputCentered<PJ301MPort>(
            mm2px(Vec(95.9f, 33.33f)), module, MassDriver::PRESSURE_CV_INPUT));

        // WIDTH: knob (70, 49.33) atten (64.1, 58.67) cv (75.9, 58.67)
        addParam(createParamCentered<RoundLargeBlackKnob>(
            mm2px(Vec(70.f, 49.33f)), module, MassDriver::WIDTH_PARAM));
        addParam(createParamCentered<Trimpot>(
            mm2px(Vec(64.1f, 58.67f)), module, MassDriver::WIDTH_ATTEN_PARAM));
        addInput(createInputCentered<PJ301MPort>(
            mm2px(Vec(75.9f, 58.67f)), module, MassDriver::WIDTH_CV_INPUT));

        // MASS: knob (90, 49.33) atten (84.1, 58.67) cv (95.9, 58.67)
        addParam(createParamCentered<RoundLargeBlackKnob>(
            mm2px(Vec(90.f, 49.33f)), module, MassDriver::MASS_PARAM));
        addParam(createParamCentered<Trimpot>(
            mm2px(Vec(84.1f, 58.67f)), module, MassDriver::MASS_ATTEN_PARAM));
        addInput(createInputCentered<PJ301MPort>(
            mm2px(Vec(95.9f, 58.67f)), module, MassDriver::MASS_CV_INPUT));

        // FEEDBACK: knob (70, 76) atten (64.1, 85.33) cv (75.9, 85.33)
        addParam(createParamCentered<RoundBlackKnob>(
            mm2px(Vec(70.f, 76.f)), module, MassDriver::FEEDBACK_PARAM));
        addParam(createParamCentered<Trimpot>(
            mm2px(Vec(64.1f, 85.33f)), module, MassDriver::FEEDBACK_ATTEN_PARAM));
        addInput(createInputCentered<PJ301MPort>(
            mm2px(Vec(75.9f, 85.33f)), module, MassDriver::FEEDBACK_CV_INPUT));

        // COLLAPSE: button (90, 74) gate (90, 85.33)
        addParam(createParamCentered<TL1105>(
            mm2px(Vec(90.f, 74.f)), module, MassDriver::COLLAPSE_PARAM));
        addInput(createInputCentered<PJ301MPort>(
            mm2px(Vec(90.f, 85.33f)), module, MassDriver::COLLAPSE_INPUT));

        // V/OCT IN (80, 99.33)
        addInput(createInputCentered<PJ301MPort>(
            mm2px(Vec(80.f, 99.33f)), module, MassDriver::VOCT_INPUT));

        // ── Outputs (y = 342px = 114mm) ──────────────────────────────
        addOutput(createOutputCentered<PJ301MPort>(
            mm2px(Vec(60.f, 114.f)), module, MassDriver::OUT_L_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(
            mm2px(Vec(70.f, 114.f)), module, MassDriver::AUX_L_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(
            mm2px(Vec(80.f, 114.f)), module, MassDriver::SUM_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(
            mm2px(Vec(90.f, 114.f)), module, MassDriver::AUX_R_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(
            mm2px(Vec(100.f, 114.f)), module, MassDriver::OUT_R_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(
            mm2px(Vec(80.f, 126.f)), module, MassDriver::VOCT_OUTPUT));
    }

    void draw(const DrawArgs& args) override {
        ModuleWidget::draw(args);
        nvgFontSize(args.vg, 11.f);
        nvgFontFaceId(args.vg, APP->window->uiFont->handle);
        nvgTextLetterSpacing(args.vg, 3.f);
        nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(args.vg, box.size.x * 0.5f, 13.f, "MASS DRIVER", nullptr);
    }
};

Model* modelMassDriver = createModel<MassDriver, MassDriverWidget>("MassDriver");
