#pragma once
#include "renderDefault.h"
class RenderPass {
protected:
	unsigned indexInPass;
	unsigned FBO;
	Shader* shader;

public:
	// if want to use FBO which != 0, can initialize FBO in initTexture()
	RenderPass(unsigned indexInPass)
		:indexInPass(indexInPass), FBO(0), shader(nullptr) {};
	virtual ~RenderPass() {}
	/* =================
	* before render loop
	*/
	virtual void initGlobalSettings() = 0;
	virtual void initShader() = 0;
	virtual void initTexture() = 0;
	virtual void initScene() = 0;
	/* =================
	* in render loop
	*/
	virtual void Render() = 0;
};