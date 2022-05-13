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
	// samplesN on theta & phi(cubemap)
	unsigned samplesN;
	// һ�����ٸ�
	unsigned uGridTextureSize;
	float* getSamplesRandom(unsigned samplesN);
	void genSampleVAO();
public:
	LightInjectionPass(int indexInPass) : RenderPass(indexInPass) {
		//this->samplesN = 50;
		//this->uGridTextureSize = 50;
		this->samplesN = 25;
		this->uGridTextureSize = 10;
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
	// get
	unsigned getSamplesN();
	unsigned getUGridTextureSize();
};
/* =====lightPropogation Pass=====
* ������pass
* opengl-settings: �ر���Ȳ���
* opengl-input: ����ע��grid��������ֵ��grid���ӣ���samplesIdxInGridTexture���벢��CPUԤ����
* opengl-output: ʹ��imageTextures���д洢������ 6*RGB 3D gridTexture
*/
class LightPropogationPass : public RenderPass {
private:
	// ������������������=uGridTextureSize*propogationRate
	unsigned propogationCount;
	// ˥����ֵ
	float propogationGate;
	// ����Ⱦʱ��̬����
	unsigned uniquedPoints;
	// ����һpass�̳�
	unsigned samplesN;
	// һ�����ٸ�
	unsigned uGridTextureSize;

	unsigned getVAOFromSamplesIdxGridTex(int count);
	static bool compareU32vec3(const glm::u32vec3 v1, const glm::u32vec3 v2);
	static bool compareVec3(const glm::vec3 v1, const glm::vec3 v2);
public:
	LightPropogationPass(int indexInPass, unsigned samplesN, unsigned uGridTextureSize, float propogationRate = 0.3f, float propogationGate = 0.005f) :
		RenderPass(indexInPass), samplesN(samplesN), uGridTextureSize(uGridTextureSize), propogationGate(propogationGate) {
		if (propogationRate < 0.f) {
			cout << "ERROR: propogationRate < 0.f in LightPropogationPass" << endl;
		}
		this->uniquedPoints = 0;
		this->propogationCount = unsigned(floor(float(uGridTextureSize) * propogationRate + 0.5f));
		// 6 * (5^12)
		this->propogationCount = this->propogationCount <= 5 ? this->propogationCount : 5;
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