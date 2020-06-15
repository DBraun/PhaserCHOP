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
		info->customOPInfo.authorEmail->setString("github.com/DBraun");

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
	ginfo->cookEveryFrameIfAsked = false;
	ginfo->timeslice = false;
	ginfo->inputMatchIndex = 0;
}

bool
PhaserCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// return false to just match the first input's info entirely
	return false;
}

void
PhaserCHOP::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1)
{
	// This function doesn't matter because getOutputInfo returns false to match the first input CHOP.
	// name->setString("chan1");
}

float
PhaserCHOP::clamp(double val, double lower, double upper)
{
	return val <= lower ? lower : val >= upper ? upper : val;
	// return std::max(std::min(val, upper), lower); // alternative equivalent version
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
float
PhaserCHOP::phaser(double t, double _phase, double edge)
{
	// safety checks because phase must be [0-1].
	float phase = clamp(_phase, 0.f, 1.f);
	// but we will assume t has been clamped to [0,1] before entering this function.
	// We will also assume edge is greater than and not equal to 0.

	// smaller edge corresponds to sharper separation according
	// to differences in phase
	return clamp((-1.f + phase + t*(1.f + edge)) / edge, 0.f, 1.f);
}

void
PhaserCHOP::execute(CHOP_Output* output,
	const OP_Inputs* inputs,
	void* reserved)
{	

	// remove errors
	myError = "";

	// Edge can't be zero. We'll rely on the Parameter settings to prevent this.
	double Edge = inputs->getParDouble("Edge");
	Edge = std::max(smallestDouble, Edge);
	int numInputs = inputs->getNumInputs();

	bool canGetEdge = false;

	const OP_CHOPInput* edgeInput;
	if (numInputs > 2)
	{
		edgeInput = inputs->getInputCHOP(2);
		if (edgeInput->numSamples > 0 && edgeInput->numChannels > 0) {
			canGetEdge = true;
		}
	}

	if (numInputs > 1)
	{
		const OP_CHOPInput* phaseInput;
		const OP_CHOPInput* timeInput;
		try
		{
			phaseInput = inputs->getInputCHOP(0);
			timeInput = inputs->getInputCHOP(1);
		}
		catch (std::exception& e)
		{
			myError = (char*) e.what();
			return;
		}

		// For PhaserCHOP to work, at a minimum we need both a valid
		// phaseInput and valid timeInput.
		if (!phaseInput || !timeInput)
		{
			// We have at least two inputs,
			// but one of them isn't phase and one of them isn't time.
			// So just return.
			return;
		}

		float t = 0.f;
		// Can we safely access the time input from the second input chop?
		if (timeInput->numChannels > 0 && timeInput->numSamples > 0)
		{
			// Get the latest sample in the time input because PhaserCHOP doesn't
			// yet support timeslicing.
			t = timeInput->getChannelData(0)[timeInput->numSamples-1];
			t = clamp(t, 0., 1.);
		}

		int numChannels = output->numChannels;
		int numSamples = output->numSamples;

		for (int j = 0; j < numSamples; j++)
		{
			for (int i = 0; i < numChannels; i++)
			{
				if (canGetEdge)
				{
					Edge = edgeInput->getChannelData(std::min(i, edgeInput->numChannels-1))[std::min(j, edgeInput->numSamples - 1)];

					// Edge must be greater than zero.
					Edge = std::max(smallestDouble, Edge);
				}

				float phase = phaseInput->getChannelData(i)[j];
				
				output->channels[i][j] = phaser(t, phase, Edge);
			}
		}
	}
	else if (numInputs == 1)
	{
		// copy over the data, but linear clamp it.

		const OP_CHOPInput* phaseInput;
		try
		{
			phaseInput = inputs->getInputCHOP(0);
		}
		catch (std::exception& e)
		{
			myError = (char*)e.what();
			return;
		}

		if (!phaseInput)
		{
			// we have 1 input but it's not the phase input
			return;
		}

		int numChannels = output->numChannels;
		int numSamples = output->numSamples;

		for (int j = 0; j < numSamples; j++)
		{
			for (int i = 0; i < numChannels; i++)
			{
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
		np.minSliders[0] = smallestDouble;
		np.maxSliders[0] =  10.0;

		np.clampMins[0] = true;
		np.minValues[0] = smallestDouble;
		
		OP_ParAppendResult res = manager->appendFloat(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void 
PhaserCHOP::pulsePressed(const char* name, void* reserved1)
{
}

