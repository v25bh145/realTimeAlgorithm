#include "renderPass.h"

void RenderPass::initLastPass(vector<unsigned> passVAO, vector<unsigned> passTexture)
{
	this->passVAO = passVAO;
	this->passTexture = passTexture;
}
