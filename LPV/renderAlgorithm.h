#pragma once

#include "default.h"
#include "renderPass.h"

class RenderAlgorithm {
private:
	vector<RenderPass*> rps;
	unsigned initVAO;
public:
	RenderAlgorithm() {
		// implement
	};
	void globalSettings();
	void prepareRendering();
	void renderingOnce();
};