// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>

// ============================================================
// SWARM CORE — Bio-Acoustic Insect Sample Engine
// 18 HP | Amplified Futures
//
// Sample bank: loads mono 44.1 kHz PCM WAV files from
//   res/insects/insectset32/cicadidae/
//   res/insects/insectset32/orthoptera/
// (InsectSet32, CC-BY 4.0 / Zenodo 7072196)
//
// When the sample folder is absent, falls back to a noise-burst
// so the module is always functional.
//
// Modes:
//   SPECIMEN — single voice pitched playback
//   SWARM    — up to 8 concurrent voices, detuned + time-scattered
// ============================================================

// ─── Minimal PCM WAV loader ──────────────────────────────────
// Handles 16-bit and float32 mono/stereo PCM only.
// Clips to maxFrames so loading many long recordings stays memory-safe.
static bool loadWavMono(const std::string& path,
                         std::vector<float>& out, int& sampleRate,
                         uint32_t maxFrames = 220500) { // 5 s @ 44.1 kHz
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return false;
    auto readU32 = [&]() -> uint32_t {
        uint8_t b[4]; fread(b, 1, 4, f);
        return b[0] | (b[1]<<8) | (b[2]<<16) | (b[3]<<24);
    };
    auto readU16 = [&]() -> uint16_t {
        uint8_t b[2]; fread(b, 1, 2, f);
        return b[0] | (b[1]<<8);
    };
    char hdr[4];
    fread(hdr, 1, 4, f);
    if (memcmp(hdr, "RIFF", 4) != 0) { fclose(f); return false; }
    readU32(); // chunk size
    fread(hdr, 1, 4, f);
    if (memcmp(hdr, "WAVE", 4) != 0) { fclose(f); return false; }
    uint16_t audioFmt = 0, numCh = 1, bitsPerSample = 16;
    uint32_t rateHz = 44100, dataSize = 0;
    while (!feof(f)) {
        char id[4]; if (fread(id, 1, 4, f) != 4) break;
        uint32_t sz = readU32();
        if (memcmp(id, "fmt ", 4) == 0) {
            audioFmt     = readU16();
            numCh        = readU16();
            rateHz       = readU32();
            readU32(); readU16(); // byte-rate, block align
            bitsPerSample = readU16();
            if (sz > 16) fseek(f, sz - 16, SEEK_CUR);
        } else if (memcmp(id, "data", 4) == 0) {
            dataSize = sz;
            break;
        } else {
            fseek(f, sz, SEEK_CUR);
        }
    }
    if (dataSize == 0 || (audioFmt != 1 && audioFmt != 3)) {
        fclose(f); return false;
    }
    sampleRate = (int)rateHz;
    if (audioFmt == 3 && bitsPerSample == 32) {
        uint32_t nFrames = std::min(dataSize / (numCh * 4), maxFrames);
        out.resize(nFrames);
        const size_t sampleCount = static_cast<size_t>(nFrames) * static_cast<size_t>(numCh);
        std::vector<float> buf(sampleCount);
        fread(buf.data(), 4, sampleCount, f);
        for (uint32_t i = 0; i < nFrames; i++) out[i] = buf[static_cast<size_t>(i) * numCh];
    } else {
        uint32_t nFrames = std::min(dataSize / (numCh * 2), maxFrames);
        out.resize(nFrames);
        const size_t sampleCount = static_cast<size_t>(nFrames) * static_cast<size_t>(numCh);
        std::vector<int16_t> buf(sampleCount);
        fread(buf.data(), 2, sampleCount, f);
        const float inv = 1.f / 32768.f;
        for (uint32_t i = 0; i < nFrames; i++) out[i] = buf[static_cast<size_t>(i) * numCh] * inv;
    }
    fclose(f);
    return !out.empty();
}

// ─── Sample bank ─────────────────────────────────────────────
struct SampleEntry { std::vector<float> data; int sr = 44100; std::string name; };

static bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Recursively collect WAV paths from dir, up to maxDepth directory levels deep.
// Skips __MACOSX resource-fork directories produced by macOS zip tools.
static void collectWavs(const std::string& dir, std::vector<std::string>& out, int maxDepth) {
    if (maxDepth < 0) return;
    auto entries = rack::system::getEntries(dir);
    for (auto& p : entries) {
        // Skip macOS metadata directories
        if (p.find("__MACOSX") != std::string::npos) continue;
        if (rack::system::isDirectory(p)) {
            collectWavs(p, out, maxDepth - 1);
        } else if (endsWith(p, ".wav") || endsWith(p, ".WAV")) {
            out.push_back(p);
            if (out.size() >= 512) return;
        }
    }
}

static std::vector<SampleEntry> loadBankFromDir(const std::string& dir) {
    std::vector<std::string> paths;
    collectWavs(dir, paths, 4); // up to 4 levels deep
    std::sort(paths.begin(), paths.end());

    std::vector<SampleEntry> bank;
    bank.reserve(std::min(paths.size(), size_t(64)));
    for (const auto& p : paths) {
        if (bank.size() >= 64) break;
        SampleEntry e;
        e.name = rack::system::getFilename(p);
        if (loadWavMono(p, e.data, e.sr)) bank.push_back(std::move(e));
    }
    return bank;
}

// ─── Synthesis voice ─────────────────────────────────────────
struct Voice {
    float phase = 0.f;
    float speed = 1.f;
    float env   = 0.f;
    float decayCoef = 0.f;
    int   sampleIdx = 0;
    bool  active = false;

    void trigger(int idx, float speedRatio, float decay) {
        sampleIdx = idx;
        phase = 0.f;
        speed = speedRatio;
        env   = 1.f;
        decayCoef = decay;
        active = true;
    }

    float tick(const std::vector<float>& data) {
        if (!active || data.empty()) return 0.f;
        size_t n = data.size();
        size_t ia = (size_t)phase % n;
        size_t ib = (ia + 1) % n;
        float frac = phase - (float)(size_t)phase;
        float s = data[ia] + frac * (data[ib] - data[ia]);
        phase += speed;
        if ((size_t)phase >= n) active = false;
        env *= decayCoef;
        return s * env;
    }
};

static const int NUM_VOICES = 8;
static const float INV_SQRT_N = 0.35355f; // 1/√8

struct AFOrangeLightSC : GrayModuleLightWidget {
    AFOrangeLightSC() { addBaseColor(nvgRGB(0xFF, 0x4A, 0x0E)); }
};

struct SwarmCore : Module {
    enum ParamId {
        SPECIMEN_PARAM,  // sample select (0–1)
        PITCH_PARAM,     // pitch offset in semitones (-24..+24)
        DENSITY_PARAM,   // swarm voice count (1–8)
        SCATTER_PARAM,   // timing scatter
        DETUNE_PARAM,    // voice detune amount
        DECAY_PARAM,     // envelope decay
        // attenuverters
        PITCH_ATT_PARAM,
        DENSITY_ATT_PARAM,
        SCATTER_ATT_PARAM,
        DETUNE_ATT_PARAM,
        // mode button
        MODE_PARAM,
        // appended - never insert above this line (Rack serialises by position)
        SPECIMEN_ATT_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        TRIG_INPUT,
        VOCT_INPUT,
        DENSITY_INPUT,
        SCATTER_INPUT,
        DETUNE_INPUT,
        // appended - never insert above this line (Rack serialises by position)
        CV_INPUT,
        SPECIMEN_CV_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        OUT_L_OUTPUT,
        OUT_R_OUTPUT,
        // appended - never insert above this line
        CV_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        ACTIVE_LIGHT,
        SWARM_LIGHT,
        LIGHTS_LEN
    };

    // Sample bank — populated by a background thread spawned in the constructor.
    // bankReady uses acquire/release semantics: once true, bank is safe to read
    // from the audio thread without a lock (no further writes ever occur).
    std::vector<SampleEntry> bank;
    std::atomic<bool> bankReady{false};
    bool noSamples = false;
    std::thread loadThread;

    // Voices
    Voice  voices[NUM_VOICES];
    // Pan spread for stereo swarm (fixed cosine-law)
    float  voicePan[NUM_VOICES] = {-1.f, -.71f, -.33f, 0.f, 0.f, .33f, .71f, 1.f};

    // Trigger edge detection
    dsp::SchmittTrigger trigIn;
    float eventPhase = 0.f;   // internal DENSITY clock, used when TRIG is unpatched
    // Scatter delay accumulators (samples)
    float  scatterAcc[NUM_VOICES] = {};
    float  scatterDelay[NUM_VOICES] = {};

    SwarmCore() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(SPECIMEN_PARAM,  0.f, 1.f, 0.f, "Specimen",   "");
        configParam(PITCH_PARAM,   -24.f, 24.f, 0.f, "Pitch",  " st");
        configParam(DENSITY_PARAM,  0.f, 1.f, 0.5f,"Density",   "");
        configParam(SCATTER_PARAM,  0.f, 1.f, 0.1f,"Scatter",   "");
        configParam(DETUNE_PARAM,   0.f, 1.f, 0.2f,"Detune",    "");
        configParam(DECAY_PARAM,    0.f, 1.f, 0.5f,"Decay",     "");
        configParam(PITCH_ATT_PARAM,   -1.f, 1.f, 0.f, "Pitch CV atten");
        configParam(DENSITY_ATT_PARAM, -1.f, 1.f, 0.f, "Density CV atten");
        configParam(SCATTER_ATT_PARAM, -1.f, 1.f, 0.f, "Scatter CV atten");
        configParam(DETUNE_ATT_PARAM,  -1.f, 1.f, 0.f, "Detune CV atten");
        configSwitch(MODE_PARAM, 0.f, 1.f, 0.f, "Mode", {"Specimen", "Swarm"});
        configParam(SPECIMEN_ATT_PARAM, -1.f, 1.f, 0.f, "Specimen attenuverter");
        configInput(SPECIMEN_CV_INPUT, "Specimen select CV");
        configInput(TRIG_INPUT,    "Trigger");
        configInput(VOCT_INPUT,    "V/OCT");
        configInput(DENSITY_INPUT, "Density CV");
        configInput(SCATTER_INPUT, "Scatter CV");
        configInput(DETUNE_INPUT,  "Detune CV");
        configInput(CV_INPUT,      "Decay CV");
        configOutput(OUT_L_OUTPUT, "Out L");
        configOutput(OUT_R_OUTPUT, "Out R");
        configOutput(CV_OUTPUT,    "Swarm envelope CV");

        // Load samples on a background thread so the audio thread is never blocked.
        // bankReady (acquire/release) acts as the publication fence: once set,
        // `bank` and `noSamples` are safe to read from process() without a lock.
        std::string base = asset::plugin(pluginInstance, "res/insects/insectset32");
        loadThread = std::thread([this, base]() {
            std::vector<SampleEntry> loaded;
            for (const char* sub : {"cicadidae", "orthoptera"}) {
                auto partial = loadBankFromDir(base + "/" + sub);
                for (auto& e : partial) loaded.push_back(std::move(e));
            }
            bank     = std::move(loaded);
            noSamples = bank.empty();
            bankReady.store(true, std::memory_order_release);
        });
    }

    ~SwarmCore() {
        if (loadThread.joinable()) loadThread.detach();
    }

    void process(const ProcessArgs& args) override {
        // Output silence while the background load thread is running.
        if (!bankReady.load(std::memory_order_acquire)) {
            outputs[OUT_L_OUTPUT].setVoltage(0.f);
            outputs[OUT_R_OUTPUT].setVoltage(0.f);
            return;
        }

        bool swarmMode = params[MODE_PARAM].getValue() > 0.5f;
        lights[SWARM_LIGHT].setBrightness(swarmMode ? 1.f : 0.f);

        // V/OCT → playback speed ratio
        float voctBase = inputs[VOCT_INPUT].getVoltage();
        float pitchSt  = params[PITCH_PARAM].getValue()
                        + params[PITCH_ATT_PARAM].getValue() * inputs[VOCT_INPUT].getVoltage() * 2.4f;
        float baseSpeed = std::pow(2.f, (voctBase + pitchSt / 12.f));

        // Select specimen
        int numSamples = (int)bank.size();
        int sampleIdx = 0;
        if (numSamples > 0) {
            float sel = clamp(params[SPECIMEN_PARAM].getValue()
                            + params[SPECIMEN_ATT_PARAM].getValue()
                              * inputs[SPECIMEN_CV_INPUT].getVoltage() / 10.f,
                              0.f, 1.f);
            sampleIdx = (int)(sel * (numSamples - 1));
        }
        int nativesr = (numSamples > 0) ? bank[sampleIdx].sr : 44100;
        float srRatio = (float)nativesr / args.sampleRate;

        // Density + scatter + detune
        float density = clamp(params[DENSITY_PARAM].getValue()
            + params[DENSITY_ATT_PARAM].getValue() * inputs[DENSITY_INPUT].getVoltage() / 10.f,
            0.f, 1.f);
        float scatter = clamp(params[SCATTER_PARAM].getValue()
            + params[SCATTER_ATT_PARAM].getValue() * inputs[SCATTER_INPUT].getVoltage() / 10.f,
            0.f, 1.f);
        float detune  = clamp(params[DETUNE_PARAM].getValue()
            + params[DETUNE_ATT_PARAM].getValue() * inputs[DETUNE_INPUT].getVoltage() / 10.f,
            0.f, 1.f);

        // Decay: 10ms..3s
        // DECAY is the one control with no satellite CV, so the general CV input
        // modulates it: +/-5V spans the full knob range, unipolar patches still work.
        float decayKnob = params[DECAY_PARAM].getValue();
        if (inputs[CV_INPUT].isConnected())
            decayKnob = clamp(decayKnob + inputs[CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        float decaySecs = std::exp2(-6.f + decayKnob * 7.8f);
        float decayCoef = std::exp(-args.sampleTime / decaySecs);

        int numVoices = swarmMode ? std::max(1, (int)(density * NUM_VOICES)) : 1;

        // Trigger.
        //
        // With nothing patched into TRIG the module used to be silent no matter
        // where DENSITY sat, which reads as a broken module rather than as a
        // module waiting for a clock. DENSITY now drives an internal event
        // clock so Swarm Core sounds standalone; a patched TRIG takes over
        // completely and the internal clock stops.
        bool fire;
        if (inputs[TRIG_INPUT].isConnected()) {
            fire = trigIn.process(inputs[TRIG_INPUT].getVoltage(), 0.1f, 1.f);
            eventPhase = 0.f;
        } else {
            // 0.5 Hz at DENSITY 0 through 30 Hz at DENSITY 1 — an occasional
            // chirp up to a continuous swarm. Squared so the low end has travel.
            const float rate = 0.5f + density * density * 29.5f;
            eventPhase += args.sampleTime * rate;
            fire = (eventPhase >= 1.f);
            if (fire)
                eventPhase -= 1.f;
        }

        if (fire) {
            for (int v = 0; v < numVoices; v++) {
                // Scatter delay in samples
                scatterDelay[v] = scatter * args.sampleRate * 0.25f * ((float)v / numVoices);
                scatterAcc[v]   = 0.f;
                // Per-voice detuning (cents)
                float detuneSt = 0.f;
                if (v > 0 && swarmMode) {
                    float spread = (v - numVoices * 0.5f) / numVoices;
                    detuneSt = spread * detune * 0.5f; // ±0.25st max
                }
                float speed = baseSpeed * srRatio * std::pow(2.f, detuneSt / 12.f);
                // Immediate trigger unless scattered
                if (scatter < 0.01f || v == 0) {
                    voices[v].trigger(sampleIdx, speed, decayCoef);
                    scatterDelay[v] = 0.f;
                }
            }
        }

        // Advance scatter accumulators — trigger delayed voices
        for (int v = 1; v < numVoices; v++) {
            if (scatterDelay[v] > 0.f && !voices[v].active) {
                scatterAcc[v] += 1.f;
                if (scatterAcc[v] >= scatterDelay[v]) {
                    float detuneSt = 0.f;
                    float spread = (v - numVoices * 0.5f) / numVoices;
                    detuneSt = spread * detune * 0.5f;
                    float speed = baseSpeed * srRatio * std::pow(2.f, detuneSt / 12.f);
                    voices[v].trigger(sampleIdx, speed, decayCoef);
                    scatterDelay[v] = 0.f;
                }
            }
        }

        // Sum voices to stereo
        float outL = 0.f, outR = 0.f;
        float envSum = 0.f;
        bool anyActive = false;
        for (int v = 0; v < NUM_VOICES; v++) {
            if (!voices[v].active) continue;
            anyActive = true;
            envSum += voices[v].env;
            float s;
            if (noSamples) {
                // Noise burst fallback
                s = voices[v].env * (2.f * random::uniform() - 1.f);
                voices[v].env *= decayCoef;
                if (voices[v].env < 1e-5f) voices[v].active = false;
            } else {
                // Index the bank directly. Binding this through a ternary with
                // an empty vector on the other arm made the conditional a
                // prvalue, so the whole sample buffer was copy-constructed on
                // the heap once per active voice per sample - an unbounded
                // allocation on the audio thread.
                s = voices[v].tick(bank[voices[v].sampleIdx].data);
            }
            float pan = voicePan[v];
            float gainL = std::cos((pan + 1.f) * 0.25f * M_PI);
            float gainR = std::sin((pan + 1.f) * 0.25f * M_PI);
            outL += s * gainL;
            outR += s * gainR;
        }
        // Normalise + output (±5V)
        float norm = swarmMode ? INV_SQRT_N : 1.f;
        outputs[OUT_L_OUTPUT].setVoltage(clamp(outL * norm * 5.f, -10.f, 10.f));
        outputs[OUT_R_OUTPUT].setVoltage(clamp(outR * norm * 5.f, -10.f, 10.f));
        // Unipolar 0-10V swarm envelope: the summed voice activity, so the CV
        // tracks how much of the swarm is currently sounding.
        outputs[CV_OUTPUT].setVoltage(clamp(envSum * INV_SQRT_N, 0.f, 1.f) * 10.f);
        lights[ACTIVE_LIGHT].setBrightness(anyActive ? 1.f : 0.f);
    }
};

struct SwarmCoreWidget : ModuleWidget {
    SwarmCoreWidget(SwarmCore* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/SwarmCore.svg")));

        // Screws

        // Widget positions are in millimetres for Rack's mm2px() helper.
        // Labels row 1: SPECIMEN / PITCH
        // 14 HP = 71.12 mm. Rows are knob / attenuverter / CV, then one I/O row.
        // The old layout put TRIG (108), DETUNE CV (112) and OUT L (122) all at
        // x=22 - three 10.7 mm ports inside 14 mm, so they overlapped on screen.
        // 18 HP = 91.44 mm. At 14 HP the control count needed ~130 mm of height
        // against a 128.5 mm panel, which is why the original overlapped ports.
        // The extra width lets all six I/O jacks share one row, reclaiming the
        // vertical space a second row plus its labels would have cost.
        const float x1 = 24.f, x2 = 67.f, xMid = 45.7f;
        const float ky1 = 23.f, ky2 = 60.f, ky3 = 98.f;
        const float attenDy = 10.f, cvDy = 20.f;
        // Row 3's attenuverter sits beside its knob; its CV joins the I/O row.
        const float xAtten3 = 36.5f;
        const float jy = 113.f;
        const float jx[6] = {10.f, 24.3f, 38.6f, 52.9f, 67.1f, 81.4f};

        // Row 1 — SPECIMEN, PITCH
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x1, ky1)), module, SwarmCore::SPECIMEN_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(x1, ky1 + attenDy)), module, SwarmCore::SPECIMEN_ATT_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, ky1 + cvDy)), module, SwarmCore::SPECIMEN_CV_INPUT));

        // PITCH keeps its own satellites. They used to sit under SPECIMEN,
        // which left this column empty and attached the attenuverter to the
        // wrong knob.
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x2, ky1)), module, SwarmCore::PITCH_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(x2, ky1 + attenDy)), module, SwarmCore::PITCH_ATT_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x2, ky1 + cvDy)), module, SwarmCore::VOCT_INPUT));

        // Row 2 — DENSITY, SCATTER
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x1, ky2)), module, SwarmCore::DENSITY_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(x1, ky2 + attenDy)), module, SwarmCore::DENSITY_ATT_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, ky2 + cvDy)), module, SwarmCore::DENSITY_INPUT));

        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x2, ky2)), module, SwarmCore::SCATTER_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(x2, ky2 + attenDy)), module, SwarmCore::SCATTER_ATT_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x2, ky2 + cvDy)), module, SwarmCore::SCATTER_INPUT));

        // Row 3 — DETUNE, DECAY (attenuverter beside the knob, CV in the input row)
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x1, ky3)), module, SwarmCore::DETUNE_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(xAtten3, ky3)), module, SwarmCore::DETUNE_ATT_PARAM));
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x2, ky3)), module, SwarmCore::DECAY_PARAM));

        // Mode button between the row-2 knobs; ACTIVE light below it
        addParam(createParamCentered<VCVLatch>(mm2px(Vec(xMid, ky2)), module, SwarmCore::MODE_PARAM));
        addChild(createLightCentered<SmallLight<AFOrangeLightSC>>(mm2px(Vec(xMid, ky2)), module, SwarmCore::SWARM_LIGHT));
        addChild(createLightCentered<SmallLight<AFOrangeLightSC>>(mm2px(Vec(xMid, ky2 + cvDy)), module, SwarmCore::ACTIVE_LIGHT));

        // I/O row: TRIG | DETUNE CV | CV IN | OUT L | OUT R | CV OUT
        addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(jx[0], jy)), module, SwarmCore::TRIG_INPUT));
        addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(jx[1], jy)), module, SwarmCore::DETUNE_INPUT));
        addInput(createInputCentered<PJ301MPort>(  mm2px(Vec(jx[2], jy)), module, SwarmCore::CV_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(jx[3], jy)), module, SwarmCore::OUT_L_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(jx[4], jy)), module, SwarmCore::OUT_R_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(jx[5], jy)), module, SwarmCore::CV_OUTPUT));
    }

    void draw(const DrawArgs& args) override {
        ModuleWidget::draw(args);
        nvgFontSize(args.vg, 11);
        nvgFillColor(args.vg, nvgRGB(0xC8, 0xC0, 0xB0));
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(args.vg, box.size.x * 0.5f, 17.f, "SWARM CORE", NULL);
    }
};

Model* modelSwarmCore = createModel<SwarmCore, SwarmCoreWidget>("SwarmCore");
