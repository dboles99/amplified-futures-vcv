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
