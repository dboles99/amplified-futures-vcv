#pragma once
// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;


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

// ─── Port morphology (AF-IDS §10) ────────────────────────────
// "Audio in / out — two distinct port morphologies. Direction never signalled
// by colour alone."
//
// v2.2.0 signalled direction by stroke colour only, on every panel. These two
// carry it by shape instead — square collar in, hexagonal collar out — leaving
// the panel colour system free to keep meaning signal *type*.
//
// Both graphics are 23.7 × 23.7, exactly PJ301M's box, so swapping them in
// changes no layout arithmetic on any of the 19 panels. They are original
// artwork: Rack's own component SVGs are not MIT and cannot be copied here.
struct AFPortIn : app::SvgPort {
    AFPortIn() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/components/AFPortIn.svg")));
    }
};

struct AFPortOut : app::SvgPort {
    AFPortOut() {
        setSvg(Svg::load(asset::plugin(pluginInstance, "res/components/AFPortOut.svg")));
    }
};
