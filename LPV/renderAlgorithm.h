#pragma once

#include "renderDefault.h"
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
		LightInjectionPass* lightInjectionPass = new LightInjectionPass(1);
		LightPropogationPass* lightPropogationPass = new LightPropogationPass(2, lightInjectionPass->getSamplesN(), lightInjectionPass->getUGridTextureSize());
		OutputCubeMapPass* outputCubeMapPass = new OutputCubeMapPass(3, "shadowWorldPosMap");
		rps.push_back(getShadowSamplePass);
		rps.push_back(lightInjectionPass);
		rps.push_back(lightPropogationPass);
		rps.push_back(outputCubeMapPass);
	};
	void globalSettings();
	void prepareRendering();
	void renderingOnce();
};