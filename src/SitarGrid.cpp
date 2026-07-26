// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"

// ============================================================
// SITAR GRID — Modal string-resonance sequencer
// 42 HP | Amplified Futures
//
// Three independent sequencing brains:
//   PITCH brain  — 8-step raga-quantised pitch sequence
//   RES brain    — 8-step resonance/timbral sequence (own clock div)
//   RIFF brain   — 8-step articulation sequence
//
// Sound engine:
//   Karplus-Strong main string
//   Jawari nonlinear bridge stage
//   8-voice sympathetic resonator bank
//   Chikari drone string
//
// JHALA BREAKDOWN engine — state machine: IDLE→BUILD→ACCEL→JHALA→LAND
// ============================================================

static const int SG_RAGA_SCALES[6][8] = {
    {0, 2, 4, 5, 7, 9, 11, 12},  // Bilawal (major)
    {0, 2, 4, 6, 7, 9, 11, 12},  // Yaman (Lydian)
    {0, 1, 4, 5, 7, 8, 11, 12},  // Bhairav
    {0, 1, 3, 5, 7, 8, 10, 12},  // Bhairavi
    {0, 2, 3, 5, 7, 9, 10, 12},  // Kafi
    {0, 2, 4, 5, 7, 9, 10, 12},  // Khamaj
};

struct AFOrangeLightSG : GrayModuleLightWidget {
    AFOrangeLightSG() { addBaseColor(nvgRGB(0xFF, 0x4A, 0x0E)); }
};

struct SitarGrid : Module {
    enum ParamId {
        // Pitch sequencer: 8 steps + length + direction
        ENUMS(PITCH_STEP_PARAM, 8),
        PITCH_LEN_PARAM,
        PITCH_DIR_PARAM,
        // Resonance sequencer: 8 steps + length + clock division
        ENUMS(RES_STEP_PARAM, 8),
        RES_LEN_PARAM,
        RES_DIV_PARAM,
        // Riff sequencer: 8 steps + length
        ENUMS(RIFF_STEP_PARAM, 8),
        RIFF_LEN_PARAM,
        // Global
        ROOT_PARAM,
        RAGA_PARAM,
        PHRASE_LEN_PARAM,
        // Sound engine
        DAMPING_PARAM,
        BRIGHTNESS_PARAM,
        MEEND_PARAM,
        // Jawari bridge
        JAWARI_PARAM,
        JAWARI_EDGE_PARAM,
        JAWARI_CHAOS_PARAM,
        // Sympathetic strings
        SYMP_DECAY_PARAM,
        SYMP_SPREAD_PARAM,
        SYMP_FEEDBACK_PARAM,
        // JHALA Breakdown engine
        BD_INT_PARAM,
        BD_ACCEL_PARAM,
        SA_GRAVITY_PARAM,
        CHIKARI_PARAM,
        ORNAMENT_PARAM,
        BD_LAND_PARAM,
        NUM_PARAMS
    };
    enum InputId {
        VOCT_INPUT,
        CLOCK_INPUT,
        RESET_INPUT,
        BD_GATE_INPUT,
        LOCK_GATE_INPUT,
        ROOT_CV_INPUT,
        JAWARI_CV_INPUT,
        NUM_INPUTS
    };
    enum OutputId {
        MAIN_L_OUTPUT,
        MAIN_R_OUTPUT,
        DRONE_OUTPUT,
        SYMP_OUTPUT,
        PITCH_CV_OUTPUT,
        GATE_OUTPUT,
        RIFF_TRIG_OUTPUT,
        RES_CV_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightId {
        ENUMS(PITCH_LIGHT, 8),
        ENUMS(RES_LIGHT, 8),
        ENUMS(RIFF_LIGHT, 8),
        BD_LIGHT,
        NUM_LIGHTS
    };

    // ── Karplus-Strong main string ──
    static const int KS_MAX = 4096;
    float ksBuf[KS_MAX] = {};
    int   ksWptr = 0;

    // ── Chikari drone string ──
    float chiBuf[KS_MAX] = {};
    int   chiWptr = 0;

    // ── Sympathetic resonator bank (8 comb filters) ──
    static const int SYMP_MAX = 2048;
    float sympBuf[8][SYMP_MAX] = {};
    int   sympWptr[8] = {};

    // ── Pitch/meend state ──
    float currentPitch  = 0.f;
    float targetPitch   = 0.f;
    float ornamentPitch = 0.f;
    float ornamentEnv   = 0.f;

    // ── Jawari chaos state ──
    float jawariChaos = 0.f;

    // ── Sequencer state ──
    int  pitchStep    = 0;
    int  resStep      = 0;
    int  riffStep     = 0;
    int  resClockCnt  = 0;
    bool locked       = false;

    // ── Breakdown state machine ──
    enum BDState { BD_IDLE, BD_BUILD, BD_ACCEL, BD_JHALA, BD_LAND };
    BDState bdState    = BD_IDLE;
    float   bdTimer    = 0.f;
    float   bdIntensity = 0.f;

    dsp::SchmittTrigger clockTrig;
    dsp::SchmittTrigger resetTrig;
    dsp::SchmittTrigger bdGateTrig;
    dsp::SchmittTrigger lockTrig;
    dsp::PulseGenerator gatePulse;
    dsp::PulseGenerator riffPulse;

    SitarGrid() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        const char* artNames[8] = {"Strike","Bend","Roll","Mute","Drone","Rest","Ornament","Return"};

        for (int i = 0; i < 8; i++) {
            configParam(PITCH_STEP_PARAM + i, 0.f, 1.f, (float)i / 7.f,
                string::f("Pitch step %d (scale degree)", i + 1));
            configParam(RES_STEP_PARAM + i, 0.f, 1.f, 0.5f,
                string::f("Resonance step %d", i + 1));
            configParam(RIFF_STEP_PARAM + i, 0.f, 7.f, 0.f,
                string::f("Riff step %d: %s", i + 1, artNames[i % 8]));
        }

        configParam(PITCH_LEN_PARAM,  1.f, 8.f, 8.f, "Pitch sequence length",  " steps");
        configParam(PITCH_DIR_PARAM,  0.f, 1.f, 0.f, "Pitch direction (0=fwd, 0.5=pend, 1=rand)");
        configParam(RES_LEN_PARAM,    1.f, 8.f, 5.f, "Resonance sequence length", " steps");
        configParam(RES_DIV_PARAM,    1.f, 4.f, 2.f, "Resonance clock division");
        configParam(RIFF_LEN_PARAM,   1.f, 8.f, 8.f, "Riff sequence length",    " steps");

        configParam(ROOT_PARAM,       -4.f, 4.f,  0.f, "Root", " V/oct");
        configParam(RAGA_PARAM,        0.f, 5.f,  1.f, "Raga/Scale (0=Bilawal 1=Yaman 2=Bhairav 3=Bhairavi 4=Kafi 5=Khamaj)");
        configParam(PHRASE_LEN_PARAM,  1.f, 32.f, 8.f, "Phrase length", " steps");

        configParam(DAMPING_PARAM,    0.f, 1.f, 0.3f, "Damping",    "%", 0.f, 100.f);
        configParam(BRIGHTNESS_PARAM, 0.f, 1.f, 0.6f, "Brightness", "%", 0.f, 100.f);
        configParam(MEEND_PARAM,      0.f, 1.f, 0.2f, "Meend (glide)");

        configParam(JAWARI_PARAM,      0.f, 1.f, 0.35f, "Jawari buzz amount");
        configParam(JAWARI_EDGE_PARAM, 0.f, 1.f, 0.50f, "Jawari edge brightness");
        configParam(JAWARI_CHAOS_PARAM,0.f, 1.f, 0.10f, "Jawari chaos/flutter");

        configParam(SYMP_DECAY_PARAM,    0.f, 1.f, 0.70f, "Sympathetic decay");
        configParam(SYMP_SPREAD_PARAM,   0.f, 1.f, 0.50f, "Sympathetic spread");
        configParam(SYMP_FEEDBACK_PARAM, 0.f, 1.f, 0.60f, "Sympathetic feedback");

        configParam(BD_INT_PARAM,     0.f, 1.f, 0.f,  "Breakdown intensity");
        configParam(BD_ACCEL_PARAM,   0.f, 1.f, 0.5f, "Breakdown acceleration");
        configParam(SA_GRAVITY_PARAM, 0.f, 1.f, 0.6f, "Sa gravity (tonic pull)");
        configParam(CHIKARI_PARAM,    0.f, 1.f, 0.3f, "Chikari density");
        configParam(ORNAMENT_PARAM,   0.f, 1.f, 0.3f, "Ornament density");
        configParam(BD_LAND_PARAM,    0.f, 1.f, 0.7f, "Sam landing strength");

        configInput(VOCT_INPUT,     "V/oct pitch");
        configInput(CLOCK_INPUT,    "Clock");
        configInput(RESET_INPUT,    "Reset");
        configInput(BD_GATE_INPUT,  "Breakdown gate");
        configInput(LOCK_GATE_INPUT,"Lock gate");
        configInput(ROOT_CV_INPUT,  "Root CV");
        configInput(JAWARI_CV_INPUT,"Jawari CV");

        configOutput(MAIN_L_OUTPUT,   "Main L");
        configOutput(MAIN_R_OUTPUT,   "Main R");
        configOutput(DRONE_OUTPUT,    "Drone/Chikari");
        configOutput(SYMP_OUTPUT,     "Sympathetic strings");
        configOutput(PITCH_CV_OUTPUT, "Pitch CV");
        configOutput(GATE_OUTPUT,     "Gate");
        configOutput(RIFF_TRIG_OUTPUT,"Riff trigger");
        configOutput(RES_CV_OUTPUT,   "Resonance CV");

        for (int i = 0; i < 8; i++) {
            configLight(PITCH_LIGHT + i, string::f("Pitch step %d", i + 1));
            configLight(RES_LIGHT   + i, string::f("Res step %d",   i + 1));
            configLight(RIFF_LIGHT  + i, string::f("Riff step %d",  i + 1));
        }
        configLight(BD_LIGHT, "Breakdown active");
    }

    // ── Karplus-Strong pluck ──────────────────────────────────
    // Fills delay line with filtered noise burst.
    void ksPluck(float freq, float vel, float brightness, float sr) {
        int len = clamp((int)(sr / std::max(20.f, freq)), 2, KS_MAX - 1);
        float lp = 0.f;
        float alpha = clamp(0.15f + brightness * 0.82f, 0.01f, 1.f);
        for (int i = 0; i < len; i++) {
            float n = random::uniform() * 2.f - 1.f;
            lp += alpha * (n - lp);
            ksBuf[(ksWptr + i) % KS_MAX] = lp * vel;
        }
    }

    float ksTick(float freq, float damping, float sr) {
        int len = clamp((int)(sr / std::max(20.f, freq)), 2, KS_MAX - 1);
        int r1  = ((ksWptr - len) % KS_MAX + KS_MAX) % KS_MAX;
        int r2  = (r1 + 1) % KS_MAX;
        float out = ksBuf[r1];
        // Two-point averaging filter = built-in string damping
        ksBuf[ksWptr] = (out + ksBuf[r2]) * 0.5f * (0.997f - damping * 0.28f);
        ksWptr = (ksWptr + 1) % KS_MAX;
        return out;
    }

    // ── Chikari (high drone string) ───────────────────────────
    void chiPluck(float freq, float vel, float sr) {
        int len = clamp((int)(sr / std::max(20.f, freq)), 2, KS_MAX - 1);
        for (int i = 0; i < len; i++)
            chiBuf[(chiWptr + i) % KS_MAX] = (random::uniform() * 2.f - 1.f) * vel;
    }

    float chiTick(float freq, float sr) {
        int len = clamp((int)(sr / std::max(20.f, freq)), 2, KS_MAX - 1);
        int r1  = ((chiWptr - len) % KS_MAX + KS_MAX) % KS_MAX;
        int r2  = (r1 + 1) % KS_MAX;
        float out = chiBuf[r1];
        chiBuf[chiWptr] = (out + chiBuf[r2]) * 0.5f * 0.992f;
        chiWptr = (chiWptr + 1) % KS_MAX;
        return out;
    }

    // ── Sympathetic comb filter (one of 8) ───────────────────
    float sympTick(int s, float freq, float fbAmt, float input, float sr) {
        int len = clamp((int)(sr / std::max(20.f, freq)), 2, SYMP_MAX - 1);
        int r   = ((sympWptr[s] - len) % SYMP_MAX + SYMP_MAX) % SYMP_MAX;
        float out = sympBuf[s][r];
        sympBuf[s][sympWptr[s]] = input * 0.04f + out * fbAmt;
        sympWptr[s] = (sympWptr[s] + 1) % SYMP_MAX;
        return out;
    }

    // ── Raga quantise (V/oct → nearest scale tone) ───────────
    float quantizePitch(float voct, float root, int raga) {
        raga = clamp(raga, 0, 5);
        float semi = (voct - root) * 12.f;
        int oct = (int)std::floor(semi / 12.f);
        float inOct = semi - oct * 12.f;
        if (inOct < 0.f) { inOct += 12.f; oct--; }
        int   best     = SG_RAGA_SCALES[raga][0];
        float bestDist = 999.f;
        for (int i = 0; i < 8; i++) {
            float d = std::abs(inOct - (float)SG_RAGA_SCALES[raga][i]);
            if (d < bestDist) { bestDist = d; best = SG_RAGA_SCALES[raga][i]; }
        }
        return root + (oct * 12.f + best) / 12.f;
    }

    void process(const ProcessArgs& args) override {
        const float sr = args.sampleRate;

        // ── Read params ──────────────────────────────────────
        float root = params[ROOT_PARAM].getValue();
        if (inputs[ROOT_CV_INPUT].isConnected())
            root = clamp(root + inputs[ROOT_CV_INPUT].getVoltage(), -4.f, 4.f);

        int raga = clamp((int)(params[RAGA_PARAM].getValue() + 0.5f), 0, 5);

        float damping   = params[DAMPING_PARAM].getValue();
        float bright    = params[BRIGHTNESS_PARAM].getValue();
        float meend     = params[MEEND_PARAM].getValue();

        float jawari    = params[JAWARI_PARAM].getValue();
        if (inputs[JAWARI_CV_INPUT].isConnected())
            jawari = clamp(jawari + inputs[JAWARI_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        float jawEdge   = params[JAWARI_EDGE_PARAM].getValue();
        float jawChaos  = params[JAWARI_CHAOS_PARAM].getValue();

        float sympDecay = params[SYMP_DECAY_PARAM].getValue();
        float sympSpread= params[SYMP_SPREAD_PARAM].getValue();
        float sympFb    = params[SYMP_FEEDBACK_PARAM].getValue();

        float bdInt     = params[BD_INT_PARAM].getValue();
        float bdAccel   = params[BD_ACCEL_PARAM].getValue();
        float saGrav    = params[SA_GRAVITY_PARAM].getValue();
        float chikariP  = params[CHIKARI_PARAM].getValue();
        float ornamentP = params[ORNAMENT_PARAM].getValue();
        float bdLand    = params[BD_LAND_PARAM].getValue();

        int pitchLen = clamp((int)(params[PITCH_LEN_PARAM].getValue() + 0.5f), 1, 8);
        int resLen   = clamp((int)(params[RES_LEN_PARAM  ].getValue() + 0.5f), 1, 8);
        int resDiv   = clamp((int)(params[RES_DIV_PARAM  ].getValue() + 0.5f), 1, 4);
        int riffLen  = clamp((int)(params[RIFF_LEN_PARAM ].getValue() + 0.5f), 1, 8);

        // ── Lock gate ────────────────────────────────────────
        if (lockTrig.process(inputs[LOCK_GATE_INPUT].getVoltage()))
            locked = !locked;

        // ── Reset ────────────────────────────────────────────
        if (resetTrig.process(inputs[RESET_INPUT].getVoltage())) {
            pitchStep = resStep = riffStep = resClockCnt = 0;
        }

        // ── Breakdown state machine ───────────────────────────
        bool bdGateHigh = inputs[BD_GATE_INPUT].getVoltage() > 2.f || bdInt > 0.05f;
        if (bdGateTrig.process(bdGateHigh ? 10.f : 0.f) && bdState == BD_IDLE)
            bdState = BD_BUILD, bdTimer = 0.f;

        bdTimer += args.sampleTime;
        switch (bdState) {
            case BD_IDLE:
                bdIntensity = 0.f;
                break;
            case BD_BUILD:
                bdIntensity = clamp(bdTimer * (1.f + bdAccel * 2.f) * 0.5f, 0.f, 0.5f);
                if (bdTimer > 2.f / (0.5f + bdAccel)) { bdState = BD_ACCEL; bdTimer = 0.f; }
                break;
            case BD_ACCEL:
                bdIntensity = clamp(0.5f + bdTimer * (1.f + bdAccel), 0.f, 0.95f);
                if (bdTimer > 1.5f) { bdState = BD_JHALA; bdTimer = 0.f; }
                break;
            case BD_JHALA:
                bdIntensity = 0.92f + 0.08f * std::sin(bdTimer * 10.f * (1.f + bdAccel));
                if (bdTimer > 3.f || !bdGateHigh) { bdState = BD_LAND; bdTimer = 0.f; }
                break;
            case BD_LAND:
                bdIntensity = clamp(1.f - bdTimer * 1.5f, 0.f, 1.f);
                if (bdTimer > 1.f) { bdState = BD_IDLE; bdTimer = 0.f; bdIntensity = 0.f; }
                break;
        }
        lights[BD_LIGHT].setSmoothBrightness(bdIntensity, args.sampleTime);

        // ── Clock: advance sequencers ─────────────────────────
        if (clockTrig.process(inputs[CLOCK_INPUT].getVoltage()) && !locked) {
            // Advance pitch + riff
            float pitchDir = params[PITCH_DIR_PARAM].getValue();
            if (pitchDir < 0.33f) {
                pitchStep = (pitchStep + 1) % pitchLen;
            } else if (pitchDir < 0.66f) {
                // pendulum (not fully implemented — use fwd for v1)
                pitchStep = (pitchStep + 1) % pitchLen;
            } else {
                pitchStep = (int)(random::uniform() * pitchLen);
            }
            riffStep = (riffStep + 1) % riffLen;

            // Resonance advances on its own clock division
            resClockCnt = (resClockCnt + 1) % resDiv;
            if (resClockCnt == 0)
                resStep = (resStep + 1) % resLen;

            // ── Read current sequencer values ──
            float pitchKnob = params[PITCH_STEP_PARAM + pitchStep].getValue();
            int   scaleDeg  = clamp((int)(pitchKnob * 8.f), 0, 7);
            float stepVoct  = root + (float)SG_RAGA_SCALES[raga][scaleDeg] / 12.f;

            // External V/OCT overrides and gets quantised to raga
            if (inputs[VOCT_INPUT].isConnected())
                stepVoct = quantizePitch(inputs[VOCT_INPUT].getVoltage(), root, raga);

            // Sa gravity — pull toward tonic with probability
            bool landing = (bdState == BD_LAND && bdLand > 0.5f);
            if (landing) {
                stepVoct = root;
            } else if (random::uniform() < saGrav * 0.25f + bdIntensity * saGrav * 0.25f) {
                stepVoct = root;
            }

            // Ornament: stochastic grace-note offset before main note
            if (random::uniform() < ornamentP * (0.5f + bdIntensity * 0.5f)) {
                ornamentPitch = stepVoct + (float)SG_RAGA_SCALES[raga][1] / 12.f;
                ornamentEnv   = 1.f;
            }

            targetPitch = stepVoct;

            // ── Articulation (RIFF brain) ──────────────────
            int artType = clamp((int)(params[RIFF_STEP_PARAM + riffStep].getValue() + 0.5f), 0, 7);
            float freq  = dsp::FREQ_C4 * dsp::exp2_taylor5(targetPitch);

            switch (artType) {
                case 0: // STRIKE — normal pluck
                    ksPluck(freq, 1.f, bright, sr);
                    gatePulse.trigger(0.05f);
                    break;
                case 1: // BEND — start a semitone below, glide up
                    currentPitch = stepVoct - 1.f / 12.f;
                    ksPluck(dsp::FREQ_C4 * dsp::exp2_taylor5(currentPitch), 0.85f, bright, sr);
                    gatePulse.trigger(0.05f);
                    break;
                case 2: // ROLL — full pluck (second micro-pluck via meend offset)
                    ksPluck(freq, 1.f, bright * 1.1f, sr);
                    gatePulse.trigger(0.05f);
                    break;
                case 3: // MUTE — deadened
                    ksPluck(freq, 0.35f, bright * 0.2f, sr);
                    break;
                case 4: // DRONE — chikari hit only, no main string
                    chiPluck(dsp::FREQ_C4 * dsp::exp2_taylor5(root + 1.f),
                             0.5f + chikariP * 0.5f, sr);
                    break;
                case 5: // REST — silence
                    break;
                case 6: // ORNAMENT — decorative pluck with grace pitch
                    ornamentPitch = stepVoct + (float)SG_RAGA_SCALES[raga][2] / 12.f;
                    ornamentEnv   = 1.8f;
                    ksPluck(freq, 0.75f, bright, sr);
                    gatePulse.trigger(0.05f);
                    break;
                case 7: // RETURN TO SA
                    targetPitch = root;
                    ksPluck(dsp::FREQ_C4 * dsp::exp2_taylor5(root), 1.1f, bright, sr);
                    gatePulse.trigger(0.05f);
                    break;
            }

            // JHALA chikari probability surge in breakdown
            if (bdState != BD_IDLE && random::uniform() < chikariP * bdIntensity * 1.5f)
                chiPluck(dsp::FREQ_C4 * dsp::exp2_taylor5(root + 1.f),
                         0.4f + bdIntensity * 0.6f, sr);

            riffPulse.trigger(0.01f);

            // Update step lights
            for (int i = 0; i < 8; i++) {
                lights[PITCH_LIGHT + i].setBrightness(i == pitchStep ? 1.f : 0.f);
                lights[RES_LIGHT   + i].setBrightness(i == resStep   ? 0.8f : 0.f);
                lights[RIFF_LIGHT  + i].setBrightness(i == riffStep  ? 1.f : 0.f);
            }
        }

        // ── Meend: glide current pitch toward target ──────────
        float glideRate = (meend < 0.005f) ? sr
                        : 1.f / (meend * 0.5f + 0.003f);
        currentPitch += (targetPitch - currentPitch)
                      * clamp(args.sampleTime * glideRate, 0.f, 1.f);

        // Ornament fade (≈33ms)
        if (ornamentEnv > 0.f)
            ornamentEnv = std::max(0.f, ornamentEnv - args.sampleTime * 30.f);

        // ── Main string DSP ───────────────────────────────────
        float effectivePitch = currentPitch;
        if (ornamentEnv > 0.f)
            effectivePitch = ornamentPitch * ornamentEnv + currentPitch * (1.f - ornamentEnv);

        float mainFreq = dsp::FREQ_C4 * dsp::exp2_taylor5(effectivePitch);
        float mainOut  = ksTick(mainFreq, damping, sr);

        // ── Jawari bridge ─────────────────────────────────────
        jawariChaos += (random::uniform() * 2.f - 1.f) * jawChaos * 0.008f;
        jawariChaos *= 0.98f;
        {
            float bias = jawEdge * 0.4f * jawari;
            float jw   = std::tanh(mainOut * (1.f + jawEdge * 3.f) + bias)
                        - std::tanh(bias);
            mainOut = mainOut * (1.f - jawari) + jw * jawari
                    + jawariChaos * mainOut * 0.15f;
        }

        // ── Chikari string ────────────────────────────────────
        float chiFreq = dsp::FREQ_C4 * dsp::exp2_taylor5(root + 1.f);
        float chiOut  = chiTick(chiFreq, sr);

        // ── Sympathetic resonator bank ────────────────────────
        float resKnob = params[RES_STEP_PARAM + resStep].getValue();
        float sympFbEff = sympFb * (0.4f + resKnob * 0.5f);
        float sympOut = 0.f;
        for (int i = 0; i < 8; i++) {
            float spread  = (i / 7.f - 0.5f) * sympSpread * 0.018f;
            float sfDeg   = (float)SG_RAGA_SCALES[raga][i];
            float sfFreq  = dsp::FREQ_C4
                          * dsp::exp2_taylor5(root + sfDeg / 12.f + spread);
            // Decay applies overall resonator ring time
            float fbAmt   = sympFbEff * (0.7f + sympDecay * 0.28f);
            sympOut += sympTick(i, sfFreq, clamp(fbAmt, 0.f, 0.99f), mainOut, sr);
        }
        sympOut /= 8.f;

        // ── Mix and output ────────────────────────────────────
        float width = 0.25f + bdIntensity * 0.2f;
        float mainL = mainOut * (1.f - width) + sympOut * width * 0.6f + chiOut * 0.12f;
        float mainR = mainOut * (1.f - width) + sympOut * width * 0.6f + chiOut * 0.20f;

        outputs[MAIN_L_OUTPUT].setVoltage(5.f * std::tanh(mainL));
        outputs[MAIN_R_OUTPUT].setVoltage(5.f * std::tanh(mainR));
        outputs[DRONE_OUTPUT ].setVoltage(5.f * std::tanh(chiOut  * 2.f));
        outputs[SYMP_OUTPUT  ].setVoltage(5.f * std::tanh(sympOut * 3.f));
        outputs[PITCH_CV_OUTPUT ].setVoltage(currentPitch);
        outputs[GATE_OUTPUT     ].setVoltage(gatePulse.process(args.sampleTime) ? 10.f : 0.f);
        outputs[RIFF_TRIG_OUTPUT].setVoltage(riffPulse.process(args.sampleTime) ? 10.f : 0.f);
        outputs[RES_CV_OUTPUT   ].setVoltage(resKnob * 10.f);
    }
};

// ============================================================
// WIDGET  (42 HP)
// ============================================================

struct SitarGridWidget : ModuleWidget {
    SitarGridWidget(SitarGrid* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/SitarGrid.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // ── Three sequencer rows ──
        // x positions (mm): 8 steps at 13mm spacing, then 2 aux at 113, 126
        const float sX[10] = {8.f,21.f,34.f,47.f,60.f,73.f,86.f,99.f,113.f,126.f};

        // PITCH BRAIN  (y=32mm knobs, lights y=22mm)
        for (int i = 0; i < 8; i++) {
            addParam(createParamCentered<RoundSmallBlackKnob>(
                mm2px(Vec(sX[i], 32.f)), module, SitarGrid::PITCH_STEP_PARAM + i));
            addChild(createLightCentered<TinyLight<AFOrangeLightSG>>(
                mm2px(Vec(sX[i], 22.f)), module, SitarGrid::PITCH_LIGHT + i));
        }
        addParam(createParamCentered<Trimpot>(mm2px(Vec(sX[8], 32.f)), module, SitarGrid::PITCH_LEN_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(sX[9], 32.f)), module, SitarGrid::PITCH_DIR_PARAM));

        // RESONANCE BRAIN  (y=54mm, lights y=44mm)
        for (int i = 0; i < 8; i++) {
            addParam(createParamCentered<RoundSmallBlackKnob>(
                mm2px(Vec(sX[i], 54.f)), module, SitarGrid::RES_STEP_PARAM + i));
            addChild(createLightCentered<TinyLight<YellowLight>>(
                mm2px(Vec(sX[i], 44.f)), module, SitarGrid::RES_LIGHT + i));
        }
        addParam(createParamCentered<Trimpot>(mm2px(Vec(sX[8], 54.f)), module, SitarGrid::RES_LEN_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(sX[9], 54.f)), module, SitarGrid::RES_DIV_PARAM));

        // RIFF BRAIN  (y=76mm, lights y=66mm)
        for (int i = 0; i < 8; i++) {
            addParam(createParamCentered<RoundSmallBlackKnob>(
                mm2px(Vec(sX[i], 76.f)), module, SitarGrid::RIFF_STEP_PARAM + i));
            addChild(createLightCentered<TinyLight<GreenLight>>(
                mm2px(Vec(sX[i], 66.f)), module, SitarGrid::RIFF_LIGHT + i));
        }
        addParam(createParamCentered<Trimpot>(mm2px(Vec(sX[8], 76.f)), module, SitarGrid::RIFF_LEN_PARAM));

        // ── Right block — engine + breakdown controls ──
        const float rX[6] = {133.f,146.f,159.f,172.f,185.f,198.f};

        // Global (y=25mm)
        addParam(createParamCentered<RoundBlackKnob>(       mm2px(Vec(rX[0], 25.f)), module, SitarGrid::ROOT_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[1], 25.f)), module, SitarGrid::RAGA_PARAM));
        addParam(createParamCentered<Trimpot>(              mm2px(Vec(rX[2], 25.f)), module, SitarGrid::PHRASE_LEN_PARAM));

        // Sound engine (y=41mm)
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[0], 41.f)), module, SitarGrid::DAMPING_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[1], 41.f)), module, SitarGrid::BRIGHTNESS_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[2], 41.f)), module, SitarGrid::MEEND_PARAM));

        // Jawari (y=55mm)
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[0], 55.f)), module, SitarGrid::JAWARI_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[1], 55.f)), module, SitarGrid::JAWARI_EDGE_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[2], 55.f)), module, SitarGrid::JAWARI_CHAOS_PARAM));

        // Sympathetic strings (y=69mm)
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[0], 69.f)), module, SitarGrid::SYMP_DECAY_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[1], 69.f)), module, SitarGrid::SYMP_SPREAD_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[2], 69.f)), module, SitarGrid::SYMP_FEEDBACK_PARAM));

        // JHALA Breakdown (y=85mm)
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[0], 85.f)), module, SitarGrid::BD_INT_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[1], 85.f)), module, SitarGrid::BD_ACCEL_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[2], 85.f)), module, SitarGrid::SA_GRAVITY_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[3], 85.f)), module, SitarGrid::CHIKARI_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[4], 85.f)), module, SitarGrid::ORNAMENT_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(  mm2px(Vec(rX[5], 85.f)), module, SitarGrid::BD_LAND_PARAM));
        addChild(createLightCentered<SmallLight<AFOrangeLightSG>>(
            mm2px(Vec(rX[5] + 8.f, 85.f)), module, SitarGrid::BD_LIGHT));

        // ── I/O rows ──────────────────────────────────────────
        const float ioX[10] = {8.f,21.f,34.f,47.f,60.f,73.f,86.f,99.f,113.f,126.f};

        // Inputs (y=106mm)
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(ioX[0],106.f)), module, SitarGrid::VOCT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(ioX[1],106.f)), module, SitarGrid::CLOCK_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(ioX[2],106.f)), module, SitarGrid::RESET_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(ioX[3],106.f)), module, SitarGrid::BD_GATE_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(ioX[4],106.f)), module, SitarGrid::LOCK_GATE_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(ioX[5],106.f)), module, SitarGrid::ROOT_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(ioX[6],106.f)), module, SitarGrid::JAWARI_CV_INPUT));


        // Outputs (y=120mm)
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(ioX[0],120.f)), module, SitarGrid::MAIN_L_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(ioX[1],120.f)), module, SitarGrid::MAIN_R_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(ioX[2],120.f)), module, SitarGrid::DRONE_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(ioX[3],120.f)), module, SitarGrid::SYMP_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(ioX[4],120.f)), module, SitarGrid::PITCH_CV_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(ioX[5],120.f)), module, SitarGrid::GATE_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(ioX[6],120.f)), module, SitarGrid::RIFF_TRIG_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(ioX[7],120.f)), module, SitarGrid::RES_CV_OUTPUT));
    }

    void draw(const DrawArgs& args) override {
        ModuleWidget::draw(args);
        if (!APP->window->uiFont) return;
        nvgFontFaceId(args.vg, APP->window->uiFont->handle);
        nvgFontSize(args.vg, 13.f);
        nvgTextLetterSpacing(args.vg, 3.f);
        nvgFillColor(args.vg, nvgRGB(0x0C, 0x10, 0x0A));
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(args.vg, box.size.x / 2.f, 11.f, "SITARGRID", NULL);
    }
};

Model* modelSitarGrid = createModel<SitarGrid, SitarGridWidget>("SitarGrid");
