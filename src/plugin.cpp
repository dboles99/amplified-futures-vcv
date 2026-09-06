// Copyright (c) 2026 Daniel Boles
// SPDX-License-Identifier: MIT
// Part of the Amplified Futures Branca Series. See LICENSE.

#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	p->addModel(modelDroneCore);
	p->addModel(modelDroneClone);
	p->addModel(modelSend);
	p->addModel(modelChoke);
	p->addModel(modelPulse);
	p->addModel(modelDrift);
	p->addModel(modelWallConductor);
	p->addModel(modelStringMassCore);
	p->addModel(modelHarmonicPressure);
	p->addModel(modelCollapseSaturator);
	p->addModel(modelFeedbackGovernor);
	p->addModel(modelMassDriver);
	p->addModel(modelSitarGrid);
	p->addModel(modelSwarmCore);
	p->addModel(modelStreetGridClock);
	p->addModel(modelCollapseEG);
	p->addModel(modelQuadVCA);
	p->addModel(modelRatchet);
	p->addModel(modelSignalBloc);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}


// ─── Transport bus membership ────────────────────────────────
//
// Which models take part in the bus, answered here because this is the only
// translation unit that sees every Model. AFExpander.hpp declares these and
// calls them before any write into a neighbour's buffer - the SDK requires
// that check, and a neighbour of another brand has a buffer of a different
// type and size, so writing into it unchecked is a stray write into somebody
// else's plugin.
//
// Readers own TransportMessage buffers on their left side. Every reader also
// forwards, so a row of them chains from the clock at its left end; Street
// Grid Clock originates the bus and reads nothing.

bool afReadsTransport(engine::Model* model) {
	return model == modelPulse
	    || model == modelRatchet
	    || model == modelSitarGrid
	    || model == modelCollapseEG
	    || model == modelDrift;
}

bool afWritesTransport(engine::Model* model) {
	return model == modelStreetGridClock || afReadsTransport(model);
}
