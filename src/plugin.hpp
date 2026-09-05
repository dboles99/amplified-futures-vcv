#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// ── AFScrew — the outline bolt ───────────────────────────────
//
// ScrewSilver is opaque and 15x15, and the four corners it occupies are
// exactly where the footer and the serial mark sit. That collision accounted
// for ten of the thirty-five label overlaps across the set, on five panels,
// and no amount of moving labels fixes it without giving up the corners.
//
// Screws are not required by Rack — they are widgets the module author
// chooses to add. Drawing the bolt as an outline keeps it structurally
// present and lets a label read straight through it.
//
// Drawn with NanoVG rather than loaded from an SVG on purpose: nanosvg
// ignores several constructs this would otherwise want, and a screw that
// silently fails to draw looks identical to one deliberately omitted.
struct AFScrew : Widget {
    AFScrew() {
        box.size = Vec(15.f, 15.f);
    }

    void draw(const DrawArgs& args) override {
        const float cx = box.size.x * 0.5f;
        const float cy = box.size.y * 0.5f;
        // 3.2px. A real M3 screw hole is 3.2mm = 4.72px radius, so this reads
        // as a rivet rather than a panel bolt - deliberately smaller, to keep
        // the corner clear for the footer and serial mark. Below about 2.5px
        // the 0.9px stroke stops reading as a fastener at default Rack zoom.
        const float r = 3.2f;
        // Concrete, well under half opacity: visible as structure, never as
        // something competing with a label passing beneath it.
        const NVGcolor ink = nvgRGBA(0x88, 0x78, 0x60, 0x66);

        nvgBeginPath(args.vg);
        for (int i = 0; i < 6; i++) {
            const float a = float(M_PI) / 6.f + i * float(M_PI) / 3.f;
            const float x = cx + r * std::cos(a);
            const float y = cy + r * std::sin(a);
            if (i == 0)
                nvgMoveTo(args.vg, x, y);
            else
                nvgLineTo(args.vg, x, y);
        }
        nvgClosePath(args.vg);
        nvgStrokeColor(args.vg, ink);
        nvgStrokeWidth(args.vg, 0.9f);
        nvgStroke(args.vg);

        // Slot, kept short so it reads as a bolt head rather than a crosshair.
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, cx - r * 0.45f, cy);
        nvgLineTo(args.vg, cx + r * 0.45f, cy);
        nvgStrokeColor(args.vg, ink);
        nvgStrokeWidth(args.vg, 0.7f);
        nvgStroke(args.vg);
    }
};

// Declare each Model, defined in each module source file
extern Model* modelDroneCore;
extern Model* modelDroneClone;
extern Model* modelSend;
extern Model* modelChoke;
extern Model* modelPulse;
extern Model* modelDrift;
extern Model* modelWallConductor;
extern Model* modelStringMassCore;
extern Model* modelHarmonicPressure;
extern Model* modelCollapseSaturator;
extern Model* modelFeedbackGovernor;
extern Model* modelMassDriver;
extern Model* modelSitarGrid;
extern Model* modelSwarmCore;
extern Model* modelStreetGridClock;
extern Model* modelCollapseEG;
extern Model* modelQuadVCA;
extern Model* modelRatchet;
extern Model* modelSignalBloc;
