#pragma once
#include "renderPass.h"
using namespace std;
using namespace glm;
// implement of LPV algorithm

#define SHADOW_WIDTH 512
#define SHADOW_HEIGHT 512

// TODO: https://blog.csdn.net/qq_16555407/article/details/84307374 通过传入image变量来实现在GLSL写入采样值
// TODO: https://blog.csdn.net/u010462297/article/details/50469950
// https://www.cnblogs.com/haihuahuang/p/12448092.html
/* =====First Pass=====
* opengl-settings: 开启深度测试
* opengl-input: 光源信息(单个点光源) & 场景信息(位置+法线+kd(颜色))
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
* 第二个pass
* opengl-settings: 关闭深度测试
* opengl-input: 采样点，包括其worldPos与flux信息
* opengl-output: 使用imageTextures进行存储的纹理 6*RGB 3D gridTexture
*/
class LightInjectionPass : public RenderPass {
private:
	// samplesN on theta & phi(cubemap)
	unsigned samplesN;
	// 一共多少格
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
* 第三个pass
* opengl-settings: 关闭深度测试
* opengl-input: 光照注入grid后所有有值的grid格子，由samplesIdxInGridTexture传入并在CPU预处理
* opengl-output: 使用imageTextures进行存储的纹理 6*RGB 3D gridTexture
*/
class LightPropogationPass : public RenderPass {
private:
	// 传播比例，传播次数=uGridTextureSize*propogationRate
	unsigned propogationCount;
	// 衰减阈值
	float propogationGate;
	// 在渲染时动态计算
	unsigned uniquedPoints;
	// 由上一pass继承
	unsigned samplesN;
	// 一共多少格
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
* opengl-settings: 开启深度测试
* opengl-input: cube材质(2D*6)
* opengl-output: 屏幕
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
* opengl-settings: 开启深度测试
* opengl-input: 2D材质
* opengl-output: 屏幕
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