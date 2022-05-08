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
	glClearColor(0.f, 0.f, 0.f, 1.0f);
	for (int index = 0; index < rps.size(); ++index) {
		RenderPass* pass = rps[index];
		pass->Render();
	}
}
