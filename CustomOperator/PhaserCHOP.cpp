/* Shared Use License: This file is owned by Derivative Inc. (Derivative) and
 * can only be used, and/or modified for use, in conjunction with 
 * Derivative's TouchDesigner software, and only if you are a licensee who has
 * accepted Derivative's TouchDesigner license or assignment agreement (which
 * also govern the use of this file).  You may share a modified version of this
 * file with another authorized licensee of Derivative's TouchDesigner software.
 * Otherwise, no redistribution or sharing of this file, with or without
 * modification, is permitted.
 */

#include "PhaserCHOP.h"

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>

#include <string>
#include <algorithm>    // std::max

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

	DLLEXPORT
		void
		FillCHOPPluginInfo(CHOP_PluginInfo* info)
	{
		// Always set this to CHOPCPlusPlusAPIVersion.
		info->apiVersion = CHOPCPlusPlusAPIVersion;

		// The opType is the unique name for this CHOP. It must start with a 
		// capital A-Z character, and all the following characters must lower case
		// or numbers (a-z, 0-9)
		info->customOPInfo.opType->setString("Phaser");

		// The opLabel is the text that will show up in the OP Create Dialog
		info->customOPInfo.opLabel->setString("Phaser CHOP");

		// Information about the author of this OP
		info->customOPInfo.authorName->setString("David Braun");
		info->customOPInfo.authorEmail->setString("github.com/dbraun");

		// This CHOP can work with 0 inputs
		info->customOPInfo.minInputs = 0;

		// It can accept up to 3 inputs
		info->customOPInfo.maxInputs = 3;
	}

DLLEXPORT
CHOP_CPlusPlusBase*
CreateCHOPInstance(const OP_NodeInfo* info)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per CHOP that is using the .dll
	return new PhaserCHOP(info);
}

DLLEXPORT
void
DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the CHOP using that instance is deleted, or
	// if the CHOP loads a different DLL
	delete (PhaserCHOP*)instance;
}

};


PhaserCHOP::PhaserCHOP(const OP_NodeInfo* info) : myNodeInfo(info)
{
}

PhaserCHOP::~PhaserCHOP()
{
}

void
PhaserCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = false;
	ginfo->timeslice = false;
	ginfo->inputMatchIndex = 0;
}

bool
PhaserCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	return false;
}

void
PhaserCHOP::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1)
{
	name->setString("chan1");
}

float PhaserCHOP::clamp(float val, float lower, float upper) {
	return val <= lower ? lower : val >= upper ? upper : val;
	//return std::max(std::min(val, upper), lower);
}

// An alternative easing function.
// A typical easing function called "ease" takes a value "t" from [0,1] and returns another value [0,1]
// where ease(0)=0, ease(1)=1.
// Imagine we have multiple unique objects that are being animated from 0 to 1 and then processed through "ease".
// We may want to stagger the way in which these objects go through "ease", and this is why we use the "phaser" function.
// Phaser takes an additional argument called "phase", which is from [0,1]. A value of 1 corresponds to an animated object that
// "ahead of the pack"; It will start animating before others. A value of 0 for phase corresponds to an object that is late; It
// will be the last to start moving. The "edge" parameter describes the cohesiveness of the pack of animated objects; A small value
// will cause the objects to go through the animation very differently, or very sharply. A small value is a sharper edge.
// The output of the phaser function will be [0,1], so these values can then be passed to any other easing function, perhaps "smoothstep",
// or "ease-in-out".
float PhaserCHOP::phaser(float t, float _phase, float edge) {

	// safety checks because phase must be [0-1].
	float phase = clamp(_phase, 0., 1.);
	// but we will assume t has been clamped to [0,1] before entering this function.
	// We will also assume edge is greater than and not equal to 0.

	// smaller edge corresponds to sharper separation according
	// to differences in phase
	return clamp((-1. + phase + t*(1. + edge)) / edge, 0., 1.);
}

void
PhaserCHOP::execute(CHOP_Output* output,
	const OP_Inputs* inputs,
	void* reserved)
{	
	double Edge = inputs->getParDouble("Edge");
	int numInputs = inputs->getNumInputs();

	bool canGetEdge = false;

	const OP_CHOPInput* edgeInput;
	if (numInputs > 2) {
		edgeInput = inputs->getInputCHOP(2);
		if (edgeInput->numSamples > 0 && edgeInput->numChannels > 0) {
			canGetEdge = true;
		}
	}

	if (numInputs > 1)
	{
		const OP_CHOPInput* phaseInput;
		const OP_CHOPInput* timeInput;
		try {
			phaseInput = inputs->getInputCHOP(0);
			timeInput = inputs->getInputCHOP(1);
		}
		catch (...) {
			return;
		}

		if (!phaseInput || !timeInput) {
			return;
		}

		float t = 0.f;
		// can we safely access the time input from the second input chop?
		if (timeInput->numChannels > 0 && timeInput->numSamples > 0) {
			t = timeInput->getChannelData(0)[timeInput->numSamples-1];
			t = clamp(t, 0., 1.);
		}

		int numChannels = output->numChannels;
		int numSamples = output->numSamples;

		for (int j = 0; j < numSamples; j++) {

			if (canGetEdge && j < edgeInput->numSamples) {
				Edge = edgeInput->getChannelData(0)[j];
				Edge = std::max(.00001, Edge);
			}

			for (int i = 0; i < numChannels; i++){

				float phase = phaseInput->getChannelData(i)[j];
				
				output->channels[i][j] = phaser(t, phase, Edge);
			}
		}
	}
	else if (numInputs == 1) {
		// copy over the data, but linear clamp it.

		const OP_CHOPInput* phaseInput;
		try {
			phaseInput = inputs->getInputCHOP(0);
		}
		catch (...) {
			return;
		}

		if (!phaseInput) {
			return;
		}

		int numChannels = output->numChannels;
		int numSamples = output->numSamples;

		for (int j = 0; j < numSamples; j++) {

			for (int i = 0; i < numChannels; i++) {

				float phase = phaseInput->getChannelData(i)[j];
				phase = clamp(phase, 0., 1.);

				output->channels[i][j] = phase;
			}
		}
	}
}

int32_t
PhaserCHOP::getNumInfoCHOPChans(void* reserved1)
{
	return 0;
}

void
PhaserCHOP::getInfoCHOPChan(int32_t index,
	OP_InfoCHOPChan* chan,
	void* reserved1)
{
}

bool		
PhaserCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 0;
	infoSize->cols = 0;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
PhaserCHOP::getInfoDATEntries(int32_t index,
	int32_t nEntries,
	OP_InfoDATEntries* entries,
	void* reserved1)
{
}

void
PhaserCHOP::setupParameters(OP_ParameterManager* manager, void* reserved1)
{
	// Edge
	{
		OP_NumericParameter	np;

		np.name = "Edge";
		np.label = "Edge";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = .0001;
		np.maxSliders[0] =  10.0;

		np.clampMins[0] = true;
		np.minValues[0] = .0001;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void 
PhaserCHOP::pulsePressed(const char* name, void* reserved1)
{
}

