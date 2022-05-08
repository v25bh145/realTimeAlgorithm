#pragma once
#include "renderPass.h"
using namespace std;
using namespace glm;
// implement of LPV algorithm

#define SHADOW_WIDTH 512
#define SHADOW_HEIGHT 512

// TODO: https://blog.csdn.net/qq_16555407/article/details/84307374 ͨ������image������ʵ����GLSLд�����ֵ
// TODO: https://blog.csdn.net/u010462297/article/details/50469950
// https://www.cnblogs.com/haihuahuang/p/12448092.html
/* =====First Pass=====
* opengl-settings: ������Ȳ���
* opengl-input: ��Դ��Ϣ(�������Դ) & ������Ϣ(λ��+����+kd(��ɫ))
* opengl-output: shadowMap & fluxMap
*/
class GetShadowSamplePass : public RenderPass {
private:
public:
	GetShadowSamplePass(int indexInPass)
		:RenderPass(indexInPass){
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
	void Render() override;
};
/* =====LightInjectionPass Pass=====
* �ڶ���pass
* opengl-settings: ������Ȳ���
* opengl-input: �����㣬������worldPos��flux��Ϣ
* opengl-output: ʹ��imageTextures���д洢������ 3*RGB 3D gridTexture
*/
class LightInjectionPass : public RenderPass {
private:
	// samplesN on theta & phi(cubemap)
	unsigned samplesN;
	unsigned uGridTextureSize;
	float* getSamplesRandom(unsigned samplesN);
public:
	LightInjectionPass(int indexInPass) : RenderPass(indexInPass) {
		//this->samplesN = 50;
		//this->uGridTextureSize = 50;
		this->samplesN = 1;
		this->uGridTextureSize = 1;
	};
	LightInjectionPass(int indexInPass, int samplesN, int uGridTextureSize) : RenderPass(indexInPass) {
		this->samplesN = samplesN;
		this->uGridTextureSize = uGridTextureSize;
	};
	virtual ~LightInjectionPass() {}
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
	void Render() override;
};
/* =====output / test Pass=====
* opengl-settings: ������Ȳ���
* opengl-input: cube����(2D*6)
* opengl-output: ��Ļ
*/
class OutputCubeMapPass : public RenderPass {
private:
	string textureName;
public:
	OutputCubeMapPass(int indexInPass, string textureName)
		:RenderPass(indexInPass), textureName(textureName) {
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
	void Render() override;
};
/* =====output / test Pass=====
* opengl-settings: ������Ȳ���
* opengl-input: 2D����
* opengl-output: ��Ļ
*/
class Output2DPass : public RenderPass {
private:
	string textureName;
public:
	Output2DPass(int indexInPass, string textureName)
		:RenderPass(indexInPass), textureName(textureName) {
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
	void Render() override;
};