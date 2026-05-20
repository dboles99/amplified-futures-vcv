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

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
