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
* opengl-settings: �ر���Ȳ���
* opengl-input: �����㣬������worldPos��flux��Ϣ
* opengl-output: ʹ��imageTextures���д洢������ 6*RGB 3D gridTexture
*/
class LightInjectionPass : public RenderPass {
private:
	// һ�����ٸ�
	int iGridTextureSize;
	const unsigned INJECTION_WIDTH = 2048;
	const unsigned INJECTION_HEIGHT = 2048;
public:
	LightInjectionPass(int indexInPass) : RenderPass(indexInPass) {
		//this->iGridTextureSize = 50;
		this->iGridTextureSize = 100;
	};
	LightInjectionPass(int indexInPass, int iGridTextureSize) : RenderPass(indexInPass) {
		this->iGridTextureSize = iGridTextureSize;
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
	// get
	unsigned getIGridTextureSize();
};
/* =====lightPropogation Pass=====
* ������pass
* opengl-settings: �ر���Ȳ���
* opengl-input: ����ע��grid��������ֵ��grid���ӣ���samplesIdxInGridTexture���벢��CPUԤ����
* opengl-output: ʹ��imageTextures���д洢������ 6*RGB 3D gridTexture
*/
class LightPropogationPass : public RenderPass {
private:
	// ������������������=iGridTextureSize*propogationRate
	unsigned propogationCount;
	// һ�����ٸ�
	int iGridTextureSize;

public:
	LightPropogationPass(int indexInPass, int iGridTextureSize, float propogationRate = 0.1f) :
		RenderPass(indexInPass), iGridTextureSize(iGridTextureSize) {
		if (propogationRate < 0.f) {
			cout << "ERROR: propogationRate < 0.f in LightPropogationPass" << endl;
		}
		this->propogationCount = unsigned(floor(float(iGridTextureSize) * propogationRate + 0.5f));
		// 6 * (5^12)
		this->propogationCount = this->propogationCount <= 10 ? this->propogationCount : 10;
	};
	virtual ~LightPropogationPass() {}
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
/* =====GBufferPass Pass=====
* opengl-settings: ������Ȳ���
* opengl-input: ���е㡢��������ӽǱ任
* opengl-output: 2D ��Ļ�ռ���ͼ ������(�������ģ�����flux) ���� ����
*/
class GBufferPass : public RenderPass {
private:
public:
	GBufferPass(int indexInPass)
		:RenderPass(indexInPass) {
	};
	virtual ~GBufferPass() {}
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
/* =====LPVOutputClass Pass=====
* opengl-settings: �ر���Ȳ���
* opengl-input: ��Ļ
* opengl-texture: 2D-��Ļ�ռ���ͼ: ������(�������ģ�����flux) ���� ����
* opengl-texture: 3D-grid texture: ������гϵ��
* opengl-output: ��ɫ
*/
class LPVOutputPass : public RenderPass {
private:
public:
	// һ�����ٸ�
	int iGridTextureSize;
	LPVOutputPass(int indexInPass, int iGridTextureSize)
		:RenderPass(indexInPass),iGridTextureSize(iGridTextureSize) {
	};
	virtual ~LPVOutputPass() {}
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
* opengl-settings: �ر���Ȳ���
* opengl-input: ��Ļ
* opengl-output: ��ɫ
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