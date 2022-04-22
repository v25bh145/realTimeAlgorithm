#pragma once
#include "renderPass.h"
// implement of LPV algorithm

// TODO: https://blog.csdn.net/qq_16555407/article/details/84307374 通过传入image变量来实现在GLSL写入采样值
// TODO: https://blog.csdn.net/u010462297/article/details/50469950

/* =====First Pass=====
* opengl-settings: 设置视口为采样点*
* opengl-input: 光源信息(点光源组+面光源组) & 场景信息(位置+法线+kd(颜色)) & 采样点位置 & 采样点数量
* opengl-output: 3张一维RGB图，维度长度=采样个数，值=3*3=9个球谐系数
*/
class GetVPLSamplePass : public RenderPass {
public:
	GetVPLSamplePass(int indexInPass)
		:RenderPass(indexInPass) {};
	virtual ~GetVPLSamplePass() {}
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