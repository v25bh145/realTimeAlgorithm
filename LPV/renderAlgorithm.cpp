#include "renderAlgorithm.h"

void RenderAlgorithm::globalSettings()
{
	// implement OR implement in pass's initGlobalSettings
}

void RenderAlgorithm::prepareRendering()
{
	for (RenderPass* pass : rps) {
		pass->initGlobalSettings();
		pass->initShader();
		pass->initTexture();
		pass->initScene();
	}
}

void RenderAlgorithm::renderingOnce()
{
	vector<unsigned> lastVAO;
	vector<unsigned> lastTexture;
	for (int index = 0; index < rps.size(); ++index) {
		RenderPass* pass = rps[index];
		pass->initLastPass(lastVAO, lastTexture);
		pass->Render();
		pass->getPassReturn(lastVAO, lastTexture);
		pass->detachPass();
	}
}
