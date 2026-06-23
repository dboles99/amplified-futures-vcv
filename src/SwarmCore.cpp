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
// 14 HP | Amplified Futures
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
        std::vector<float> buf(nFrames * numCh);
        fread(buf.data(), 4, nFrames * numCh, f);
        for (uint32_t i = 0; i < nFrames; i++) out[i] = buf[i * numCh];
    } else {
        uint32_t nFrames = std::min(dataSize / (numCh * 2), maxFrames);
        out.resize(nFrames);
        std::vector<int16_t> buf(nFrames * numCh);
        fread(buf.data(), 2, nFrames * numCh, f);
        const float inv = 1.f / 32768.f;
        for (uint32_t i = 0; i < nFrames; i++) out[i] = buf[i * numCh] * inv;
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
        PARAMS_LEN
    };
    enum InputId {
        TRIG_INPUT,
        VOCT_INPUT,
        DENSITY_INPUT,
        SCATTER_INPUT,
        DETUNE_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        OUT_L_OUTPUT,
        OUT_R_OUTPUT,
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
        configButton(MODE_PARAM, "Mode (Specimen / Swarm)");
        configInput(TRIG_INPUT,    "Trigger");
        configInput(VOCT_INPUT,    "V/OCT");
        configInput(DENSITY_INPUT, "Density CV");
        configInput(SCATTER_INPUT, "Scatter CV");
        configInput(DETUNE_INPUT,  "Detune CV");
        configOutput(OUT_L_OUTPUT, "Out L");
        configOutput(OUT_R_OUTPUT, "Out R");

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
            float sel = clamp(params[SPECIMEN_PARAM].getValue(), 0.f, 1.f);
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
        float decaySecs = std::exp2(-6.f + params[DECAY_PARAM].getValue() * 7.8f);
        float decayCoef = std::exp(-args.sampleTime / decaySecs);

        int numVoices = swarmMode ? std::max(1, (int)(density * NUM_VOICES)) : 1;

        // Trigger
        if (trigIn.process(inputs[TRIG_INPUT].getVoltage(), 0.1f, 1.f)) {
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
        bool anyActive = false;
        for (int v = 0; v < NUM_VOICES; v++) {
            if (!voices[v].active) continue;
            anyActive = true;
            const auto& data = noSamples ? std::vector<float>() : bank[voices[v].sampleIdx].data;
            float s;
            if (noSamples) {
                // Noise burst fallback
                s = voices[v].env * (2.f * random::uniform() - 1.f);
                voices[v].env *= decayCoef;
                if (voices[v].env < 1e-5f) voices[v].active = false;
            } else {
                s = voices[v].tick(data);
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
        lights[ACTIVE_LIGHT].setBrightness(anyActive ? 1.f : 0.f);
    }
};

struct SwarmCoreWidget : ModuleWidget {
    SwarmCoreWidget(SwarmCore* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/SwarmCore.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Row Y positions (mm, converted to px: 1mm = 3.78px, panel height 380px)
        // Labels row 1: SPECIMEN / PITCH
        const float x1 = 22.f, x2 = 55.f;
        const float ky1 = 55.f, ky2 = 105.f, ky3 = 155.f;
        const float jy = 310.f, jy2 = 350.f;

        // Row 1 — SPECIMEN, PITCH
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x1, ky1)), module, SwarmCore::SPECIMEN_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(x1, ky1 + 14.f)), module, SwarmCore::PITCH_ATT_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, ky1 + 28.f)), module, SwarmCore::VOCT_INPUT));

        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x2, ky1)), module, SwarmCore::PITCH_PARAM));

        // Row 2 — DENSITY, SCATTER
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x1, ky2)), module, SwarmCore::DENSITY_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(x1, ky2 + 14.f)), module, SwarmCore::DENSITY_ATT_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, ky2 + 28.f)), module, SwarmCore::DENSITY_INPUT));

        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x2, ky2)), module, SwarmCore::SCATTER_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(x2, ky2 + 14.f)), module, SwarmCore::SCATTER_ATT_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x2, ky2 + 28.f)), module, SwarmCore::SCATTER_INPUT));

        // Row 3 — DETUNE, DECAY
        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x1, ky3)), module, SwarmCore::DETUNE_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(x1, ky3 + 14.f)), module, SwarmCore::DETUNE_ATT_PARAM));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, ky3 + 28.f)), module, SwarmCore::DETUNE_INPUT));

        addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(x2, ky3)), module, SwarmCore::DECAY_PARAM));

        // Mode button
        addParam(createParamCentered<LEDButton>(mm2px(Vec(x2, ky3 + 14.f)), module, SwarmCore::MODE_PARAM));
        addChild(createLightCentered<SmallLight<AFOrangeLightSC>>(mm2px(Vec(x2, ky3 + 14.f)), module, SwarmCore::SWARM_LIGHT));

        // Trigger input
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(x1, jy)), module, SwarmCore::TRIG_INPUT));

        // Active light
        addChild(createLightCentered<SmallLight<AFOrangeLightSC>>(mm2px(Vec(x2, jy)), module, SwarmCore::ACTIVE_LIGHT));

        // Outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x1, jy2)), module, SwarmCore::OUT_L_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(x2, jy2)), module, SwarmCore::OUT_R_OUTPUT));
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
