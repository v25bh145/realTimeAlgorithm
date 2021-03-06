#pragma once
#include "renderPass.h"
using namespace std;
using namespace glm;
// implement of LPV algorithm

#define SHADOW_WIDTH 512
#define SHADOW_HEIGHT 512

// 关于image以及其原子操作的使用方法
// https://blog.csdn.net/qq_16555407/article/details/84307374
// https://blog.csdn.net/u010462297/article/details/50469950
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
* opengl-input: 立方体
* opengl-output: 使用imageTextures进行存储的纹理 6*RGB 3D gridTexture
*/
class LightInjectionPass : public RenderPass {
private:
	// 一共多少格
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
* 第三个pass
* opengl-settings: 关闭深度测试
* opengl-input: 整个格子纹理(iGridTextureSize^3)
* opengl-output: 使用imageTextures进行存储的纹理 6*RGB 3D gridTexture
*/
class LightPropogationPass : public RenderPass {
private:
	// 传播比例，传播次数=iGridTextureSize*propogationRate
	unsigned propogationCount;
	// 一共多少格
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
* opengl-settings: 开启深度测试
* opengl-input: 所有点、照相机、视角变换
* opengl-output: 2D 屏幕空间贴图 漫反射(物体表面的，不是flux) 坐标 法线
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
* opengl-settings: 关闭深度测试
* opengl-input: 屏幕
* opengl-texture: 2D-屏幕空间贴图: 漫反射(物体表面的，不是flux) 坐标 法线
* opengl-texture: 3D-grid texture: 几个球谐系数
* opengl-output: 颜色
*/
class LPVOutputPass : public RenderPass {
private:
public:
	// 一共多少格
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
* opengl-settings: 关闭深度测试
* opengl-input: 屏幕
* opengl-output: 颜色
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