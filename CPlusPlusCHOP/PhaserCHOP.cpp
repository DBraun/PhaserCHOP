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
int32_t
GetCHOPAPIVersion(void)
{
	// Always return CHOP_CPLUSPLUS_API_VERSION in this function.
	return CHOP_CPLUSPLUS_API_VERSION;
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
PhaserCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = false;
	ginfo->timeslice = false;
	ginfo->inputMatchIndex = 0;
}

bool
PhaserCHOP::getOutputInfo(CHOP_OutputInfo* info)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	return false;
}

const char*
PhaserCHOP::getChannelName(int32_t index, void* reserved)
{
	return "chan1";
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
// or "ease-out".
float PhaserCHOP::phaser(float t, float _phase, float edge) {

	// safety checks because phase must be [0-1].
	float phase = clamp(_phase, 0., 1.);
	// but we will assume t has been clamped to [0,1] before entering this function.

	// smaller edge corresponds to sharper separation according
	// to differences in phase
	return clamp((-1. + phase + t*(1. + edge)) / edge, 0., 1.);
}

void
PhaserCHOP::execute(const CHOP_Output* output,
							  OP_Inputs* inputs,
							  void* reserved)
{	
	double Edge = inputs->getParDouble("Edge");
	int numInputs = inputs->getNumInputs();

	if (numInputs > 1)
	{
		const OP_CHOPInput	*phaseInput = inputs->getInputCHOP(0);
		const OP_CHOPInput	*timeInput = inputs->getInputCHOP(1);

		float t = 0.;
		bool canGetTime = timeInput->numChannels > 0;


		int numChannels = output->numChannels;
		int numSamples = output->numSamples;

		for (int j = 0; j < numSamples; j++) {

			if (canGetTime && j < timeInput->numSamples) {
				t = timeInput->getChannelData(0)[j];
				t = clamp(t, 0., 1.);
			}

			for (int i = 0; i < numChannels; i++){

				float phase = phaseInput->getChannelData(i)[j];
				
				output->channels[i][j] = phaser(t, phase, Edge);
			}
		}
	}
	else if (numInputs == 1) {
		// copy over the data, but linear clamp it.

		const OP_CHOPInput* phaseInput = inputs->getInputCHOP(0);

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
PhaserCHOP::getNumInfoCHOPChans()
{
	return 0;
}

void
PhaserCHOP::getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan)
{
}

bool		
PhaserCHOP::getInfoDATSize(OP_InfoDATSize* infoSize)
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
										OP_InfoDATEntries* entries)
{
}

void
PhaserCHOP::setupParameters(OP_ParameterManager* manager)
{
	// Edge
	{
		OP_NumericParameter	np;

		np.name = "Edge";
		np.label = "Edge";
		np.defaultValues[0] = 1.0;
		np.minSliders[0] = .001;
		np.maxSliders[0] =  10.0;

		np.clampMins[0] = true;
		np.minValues[0] = .001;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void 
PhaserCHOP::pulsePressed(const char* name)
{
}

