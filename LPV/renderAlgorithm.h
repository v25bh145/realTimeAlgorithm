#pragma once

#include "default.h"
#include "LPVPass.h"

class RenderAlgorithm {
private:
	vector<RenderPass*> rps;
	unsigned initVAO;
public:
	RenderAlgorithm() {
		// implement
		initVAO = -1;
		GetShadowSamplePass* getShadowSamplePass = new GetShadowSamplePass(0);
		//Output2DPass* output2DPass = new Output2DPass(1);
		OutputCubeMapPass* outputCubeMapPass = new OutputCubeMapPass(1);
		rps.push_back(getShadowSamplePass);
		//rps.push_back(output2DPass);
		rps.push_back(outputCubeMapPass);
	};
	void globalSettings();
	void prepareRendering();
	void renderingOnce();
};