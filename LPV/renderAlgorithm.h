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
		LightInjectionPass* lightInjectionPass = new LightInjectionPass(1, 100, 100);
		LightPropogationPass* lightPropogationPass = new LightPropogationPass(2, lightInjectionPass->getSamplesN(), lightInjectionPass->getIGridTextureSize());
		GBufferPass* gBufferPass = new GBufferPass(3);
		LPVOutputPass* lpvOutputPass = new LPVOutputPass(4, lightInjectionPass->getIGridTextureSize());
		//OutputCubeMapPass* outputCubeMapPass = new OutputCubeMapPass(4, "shadowWorldPosMap");
		//Output2DPass* output2DPass = new Output2DPass(4, "screenDiffuseMap");
		rps.push_back(getShadowSamplePass);
		rps.push_back(lightInjectionPass);
		rps.push_back(lightPropogationPass);
		rps.push_back(gBufferPass);
		rps.push_back(lpvOutputPass);
		//rps.push_back(outputCubeMapPass);
		//rps.push_back(output2DPass);
	};
	void globalSettings();
	void prepareRendering();
	void renderingOnce();
};