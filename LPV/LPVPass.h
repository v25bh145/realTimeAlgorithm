#pragma once
#include "renderPass.h"
using namespace std;
using namespace glm;
// implement of LPV algorithm

// TODO: https://blog.csdn.net/qq_16555407/article/details/84307374 ͨ������image������ʵ����GLSLд�����ֵ
// TODO: https://blog.csdn.net/u010462297/article/details/50469950
/* =====First Pass=====
* opengl-settings: ������Ȳ���
* opengl-input: ��Դ��Ϣ(�������Դ) & ������Ϣ(λ��+����+kd(��ɫ))
* opengl-output: shadowMap & fluxMap
*/
class GetShadowSamplePass : public RenderPass {
private:
	unsigned FBO;
	vec3 pointLightPos;
	vec3 pointLightDiffuse;
	const static unsigned SHADOW_WIDTH = 512;
	const static unsigned SHADOW_HEIGHT = 512;

	unsigned shadowDepthMap, worldPosMap, fluxMap;

	unsigned quadVAO, boxVAO;
	vector<mat4> quadModelTransforms, boxModelTransforms;
public:
	GetShadowSamplePass(int indexInPass)
		:RenderPass(indexInPass), FBO(-1), pointLightPos(vec3(1.f)), pointLightDiffuse(vec3(1.f)) {
		this->quadVAO = -1;
		this->boxVAO = -1;
		this->quadModelTransforms = vector<mat4>();
		this->boxModelTransforms = vector<mat4>();

		this->shadowDepthMap = -1;
		this->worldPosMap = -1;
		this->fluxMap = -1;
	};
	virtual ~GetShadowSamplePass() {}
	/* =================
	* before render loop
	*/
	void initGlobalSettings() override;
	void initShader() override;
	void initTexture() override;
	void initScene() override;
	/* =================
	* in render loop
	*/
	// default: this->passVAO = passVAO; this->passTexture = passTexture;
	void initLastPass(vector<unsigned> passVAO, vector<unsigned> passTexture) override;
	void Render() override;
	unsigned getPassReturn(vector<unsigned>& passVAO, vector<unsigned>& passTexture) override;
	void detachPass() override;
};
/* =====output / test Pass=====
* opengl-settings: ������Ȳ���
* opengl-input: cube����(2D*6)
* opengl-output: ��Ļ
*/
class OutputCubeMapPass : public RenderPass {
private:
	unsigned skyboxVAO;
	unsigned skyboxTexture;
public:
	OutputCubeMapPass(int indexInPass)
		:RenderPass(indexInPass) {
		skyboxVAO = -1;
		skyboxTexture = -1;
	};
	virtual ~OutputCubeMapPass() {}
	/* =================
	* before render loop
	*/
	void initGlobalSettings() override;
	void initShader() override;
	void initTexture() override;
	void initScene() override;
	/* =================
	* in render loop
	*/
	// default: this->passVAO = passVAO; this->passTexture = passTexture;
	void initLastPass(vector<unsigned> passVAO, vector<unsigned> passTexture) override;
	void Render() override;
	unsigned getPassReturn(vector<unsigned>& passVAO, vector<unsigned>& passTexture) override;
	void detachPass() override;
};
/* =====output / test Pass=====
* opengl-settings: ������Ȳ���
* opengl-input: 2D����
* opengl-output: ��Ļ
*/
class Output2DPass : public RenderPass {
private:
	unsigned screenVAO;
	unsigned output2DTexture;
public:
	Output2DPass(int indexInPass)
		:RenderPass(indexInPass) {
		screenVAO = -1;
		output2DTexture = -1;
	};
	virtual ~Output2DPass() {}
	/* =================
	* before render loop
	*/
	void initGlobalSettings() override;
	void initShader() override;
	void initTexture() override;
	void initScene() override;
	/* =================
	* in render loop
	*/
	// default: this->passVAO = passVAO; this->passTexture = passTexture;
	void initLastPass(vector<unsigned> passVAO, vector<unsigned> passTexture) override;
	void Render() override;
	unsigned getPassReturn(vector<unsigned>& passVAO, vector<unsigned>& passTexture) override;
	void detachPass() override;
};