#pragma once
#include "default.h"
class RenderPass {
private:
	vector<unsigned> passTexture;
	vector<unsigned> passVAO;
	int indexInPass;

	unsigned VAO;
	unsigned shader;
public:
	RenderPass(int indexInPass)
	:indexInPass(indexInPass), passTexture(vector<unsigned>()), passVAO(vector<unsigned>()), VAO(-1), shader(-1) {};
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
	// default: this->passVAO = passVAO; this->passTexture = passTexture;
	virtual void initLastPass(vector<unsigned> passVAO, vector<unsigned> passTexture);
	virtual void Render() = 0;
	virtual unsigned getPassReturn(vector<unsigned>& passVAO, vector<unsigned>& passTexture) = 0;
	virtual void detachPass() = 0;
};