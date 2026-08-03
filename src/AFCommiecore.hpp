#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.
//
// ============================================================
// NOT COMPILED INTO THE SHIPPED PLUGIN. No module includes this header.
// Written for a three-zone redesign piloted and reverted on 2026-08-03;
// kept for a future module set. See docs/qa/next-session-plan.md.
// ============================================================
// AF-COMMIECORE for Rack
//
// The shared visual and structural vocabulary for all nineteen modules:
// palette, panel zones, the display window, and the polyphony preference.
//
// Ported from af-ui/include/af-ui/CommiecoreTokens.hpp (JUCE) so that the Rack
// modules and the commercial products read as one instrument family.
//
// Geometry authority: docs/design/commiecore-rack-geometry.md
// ============================================================

#include <rack.hpp>
#include <atomic>
#include <cmath>

using namespace rack;

namespace af {

// ── Conversion ───────────────────────────────────────────────
// Rack renders SVG at 75 DPI. Do not use 3.0 px/mm: the 1.5% error accumulates
// to 5.5 px over a full panel, which is how a port ended up off the bottom edge.
static constexpr float PX_PER_MM = 75.f / 25.4f;   // 2.952756
static constexpr float PANEL_H_MM = 128.5f;
static constexpr float HP_MM = 5.08f;

// ── Zones, in mm (spec §3) ───────────────────────────────────
namespace zone {
    static constexpr float MASTHEAD_TOP = 0.f,   MASTHEAD_BOT = 11.0f;
    static constexpr float DISPLAY_TOP  = 13.5f, DISPLAY_BOT  = 39.5f;
    static constexpr float CONTROL_TOP  = 44.0f, CONTROL_BOT  = 97.0f;
    static constexpr float PORT_TOP     = 102.0f, PORT_BOT    = 122.0f;
    static constexpr float SIDE_MARGIN  = 5.0f;
}

// ── Palette (spec §6) ────────────────────────────────────────
namespace col {
    inline NVGcolor rgb(int r, int g, int b) { return nvgRGB(r, g, b); }

    static const NVGcolor surface      = nvgRGB(0x11, 0x14, 0x10);
    static const NVGcolor raised       = nvgRGB(0x1A, 0x1F, 0x16);
    static const NVGcolor well         = nvgRGB(0x0C, 0x0F, 0x0A);
    static const NVGcolor phosphorBg   = nvgRGB(0x06, 0x08, 0x07);
    static const NVGcolor phosphor     = nvgRGB(0x39, 0xFF, 0x14);
    static const NVGcolor phosphorGlow = nvgRGB(0x0E, 0x20, 0x10);
    static const NVGcolor cream        = nvgRGB(0xE8, 0xE4, 0xD4);
    static const NVGcolor steel        = nvgRGB(0x8A, 0x90, 0x80);
    static const NVGcolor mutedOlive   = nvgRGB(0x6B, 0x7A, 0x58);
    static const NVGcolor structOrange = nvgRGB(0xC8, 0x66, 0x1A);
    static const NVGcolor safetyOrange = nvgRGB(0xE8, 0x7D, 0x00);
    static const NVGcolor border       = nvgRGB(0x1C, 0x23, 0x18);
    static const NVGcolor borderDeep   = nvgRGB(0x24, 0x2B, 0x1E);
}

// ── Display buffer ───────────────────────────────────────────
// Written by process() on the audio thread, read by draw() on the UI thread.
// A torn read shows one frame of a slightly older trace, which is invisible at
// 60 fps — worth far more than a lock in the audio path.
struct ScopeBuffer {
    static constexpr int SIZE = 512;

    float data[SIZE] = {};
    std::atomic<int> writeIndex{0};
    float peak = 0.f;

    // Decimation: at 44.1k a 512-point buffer would cover 12ms, too fast to
    // read. One sample per N keeps roughly a 200ms window on screen.
    int decimate = 0;
    int decimateRate = 16;

    void push(float v) {
        if (++decimate < decimateRate) {
            if (std::fabs(v) > peak) peak = std::fabs(v);
            return;
        }
        decimate = 0;
        int i = writeIndex.load(std::memory_order_relaxed);
        data[i] = peak > std::fabs(v) ? (v >= 0.f ? peak : -peak) : v;
        peak = 0.f;
        writeIndex.store((i + 1) % SIZE, std::memory_order_release);
    }

    void setTimebase(float sampleRate) {
        // hold the window near 200ms regardless of sample rate
        decimateRate = std::max(1, int(sampleRate * 0.2f / float(SIZE)));
    }

    void reset() {
        for (int i = 0; i < SIZE; i++) data[i] = 0.f;
        writeIndex.store(0, std::memory_order_release);
        peak = 0.f;
        decimate = 0;
    }
};

enum class DisplayMode { SCOPE, SPECTRUM, METER };

// ── Display widget ───────────────────────────────────────────
// Occupies the display zone. Draws the phosphor ground and its grid whether or
// not a module feeds it — the window is structural, not optional. Nineteen
// modules reading as one family depends on it always being there.
struct AFDisplay : TransparentWidget {
    ScopeBuffer* buffer = nullptr;
    DisplayMode mode = DisplayMode::SCOPE;
    std::string caption;

    AFDisplay(Vec pos, Vec size) {
        box.pos = pos;
        box.size = size;
    }

    void drawGround(const DrawArgs& args) {
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 2.f);
        nvgFillColor(args.vg, col::phosphorBg);
        nvgFill(args.vg);

        nvgStrokeColor(args.vg, col::borderDeep);
        nvgStrokeWidth(args.vg, 1.f);
        nvgStroke(args.vg);

        // grid: quarters horizontally, centre line vertically
        nvgStrokeColor(args.vg, col::phosphorGlow);
        nvgStrokeWidth(args.vg, 0.5f);
        for (int i = 1; i < 4; i++) {
            float x = box.size.x * i / 4.f;
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, x, 2.f);
            nvgLineTo(args.vg, x, box.size.y - 2.f);
            nvgStroke(args.vg);
        }
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, 2.f, box.size.y / 2.f);
        nvgLineTo(args.vg, box.size.x - 2.f, box.size.y / 2.f);
        nvgStroke(args.vg);
    }

    void drawScope(const DrawArgs& args) {
        if (!buffer) return;
        int w = buffer->writeIndex.load(std::memory_order_acquire);
        float midY = box.size.y / 2.f;
        float ampY = (box.size.y / 2.f) - 2.f;

        nvgBeginPath(args.vg);
        for (int i = 0; i < ScopeBuffer::SIZE; i++) {
            int idx = (w + i) % ScopeBuffer::SIZE;
            float x = box.size.x * float(i) / float(ScopeBuffer::SIZE - 1);
            float v = clamp(buffer->data[idx] / 5.f, -1.f, 1.f);
            float y = midY - v * ampY;
            if (i == 0) nvgMoveTo(args.vg, x, y);
            else        nvgLineTo(args.vg, x, y);
        }
        nvgStrokeColor(args.vg, col::phosphor);
        nvgStrokeWidth(args.vg, 1.2f);
        nvgLineCap(args.vg, NVG_ROUND);
        nvgLineJoin(args.vg, NVG_ROUND);
        nvgStroke(args.vg);
    }

    void drawMeter(const DrawArgs& args) {
        if (!buffer) return;
        float sum = 0.f;
        for (int i = 0; i < ScopeBuffer::SIZE; i++)
            sum += buffer->data[i] * buffer->data[i];
        float rms = std::sqrt(sum / float(ScopeBuffer::SIZE));
        float norm = clamp(rms / 5.f, 0.f, 1.f);

        float pad = 3.f;
        float h = (box.size.y - pad * 2.f) * norm;
        nvgBeginPath(args.vg);
        nvgRect(args.vg, pad, box.size.y - pad - h, box.size.x - pad * 2.f, h);
        nvgFillColor(args.vg, norm > 0.9f ? col::structOrange : col::phosphor);
        nvgFill(args.vg);
    }

    // Magnitude of a coarse DFT. 512 points at 32 bins is cheap enough to run
    // per frame and reads as a spectral silhouette rather than an analyser.
    void drawSpectrum(const DrawArgs& args) {
        if (!buffer) return;
        static constexpr int BINS = 32;
        float mag[BINS] = {};
        int w = buffer->writeIndex.load(std::memory_order_acquire);

        for (int k = 0; k < BINS; k++) {
            float re = 0.f, im = 0.f;
            for (int n = 0; n < ScopeBuffer::SIZE; n += 4) {
                int idx = (w + n) % ScopeBuffer::SIZE;
                float ang = 2.f * float(M_PI) * float(k) * float(n)
                          / float(ScopeBuffer::SIZE);
                re += buffer->data[idx] * std::cos(ang);
                im -= buffer->data[idx] * std::sin(ang);
            }
            mag[k] = std::sqrt(re * re + im * im) / float(ScopeBuffer::SIZE / 4);
        }

        float bw = box.size.x / float(BINS);
        for (int k = 0; k < BINS; k++) {
            float norm = clamp(mag[k] / 2.f, 0.f, 1.f);
            float h = (box.size.y - 4.f) * norm;
            nvgBeginPath(args.vg);
            nvgRect(args.vg, k * bw + 0.5f, box.size.y - 2.f - h,
                    bw - 1.f, h);
            nvgFillColor(args.vg, col::phosphor);
            nvgFill(args.vg);
        }
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1) {
            TransparentWidget::drawLayer(args, layer);
            return;
        }
        drawGround(args);
        switch (mode) {
            case DisplayMode::SCOPE:    drawScope(args);    break;
            case DisplayMode::SPECTRUM: drawSpectrum(args); break;
            case DisplayMode::METER:    drawMeter(args);    break;
        }
        TransparentWidget::drawLayer(args, layer);
    }
};

// ── Polyphony preference ─────────────────────────────────────
// Attenuverters are hidden on channels carrying a polyphonic cable. The
// parameter is never removed and its enum position never changes — only the
// widget's visibility — so every saved patch survives.
struct PolyPrefs {
    bool hideAttenOnPoly = true;

    json_t* toJson() const {
        json_t* root = json_object();
        json_object_set_new(root, "hideAttenOnPoly",
                            json_boolean(hideAttenOnPoly));
        return root;
    }

    void fromJson(json_t* root) {
        if (!root) return;
        if (json_t* j = json_object_get(root, "hideAttenOnPoly"))
            hideAttenOnPoly = json_boolean_value(j);
    }
};

// One attenuverter widget and the input whose channel count governs it.
// Explicit constructor rather than default member initialisers: the Rack SDK
// compiles at -std=c++11, where a struct with NSDMIs is not an aggregate and
// brace-initialising a vector of them fails.
struct AttenLink {
    Widget* widget;
    int inputId;
    AttenLink(Widget* w = nullptr, int id = -1) : widget(w), inputId(id) {}
};

inline void appendPolyMenu(Menu* menu, PolyPrefs* prefs) {
    if (!prefs) return;
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel("Polyphony"));
    menu->addChild(createBoolPtrMenuItem(
        "Hide attenuverters on poly channels", "", &prefs->hideAttenOnPoly));
}

// Call from ModuleWidget::step(). Cheap: a visibility compare per link.
inline void syncAttenVisibility(Module* module, const PolyPrefs& prefs,
                                const std::vector<AttenLink>& links) {
    for (const AttenLink& link : links) {
        if (!link.widget) continue;
        bool poly = module
                 && link.inputId >= 0
                 && link.inputId < (int) module->inputs.size()
                 && module->inputs[link.inputId].getChannels() > 1;
        bool wantVisible = !(prefs.hideAttenOnPoly && poly);
        if (link.widget->isVisible() != wantVisible)
            link.widget->setVisible(wantVisible);
    }
}

// ── Masthead ─────────────────────────────────────────────────
// The module name is drawn here, in NanoVG, and never as SVG text — nanosvg
// renders neither text nor fonts.
inline void drawMasthead(NVGcontext* vg, float panelW, const char* name) {
    if (!APP->window->uiFont) return;
    nvgFontFaceId(vg, APP->window->uiFont->handle);
    nvgFontSize(vg, 12.f);
    nvgTextLetterSpacing(vg, 2.4f);
    nvgFillColor(vg, col::cream);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(vg, panelW / 2.f, zone::MASTHEAD_BOT * PX_PER_MM * 0.55f,
            name, NULL);
}

} // namespace af
