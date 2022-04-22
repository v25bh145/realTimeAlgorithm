#pragma once
#include "renderPass.h"
// implement of LPV algorithm

// TODO: https://blog.csdn.net/qq_16555407/article/details/84307374 ͨ������image������ʵ����GLSLд�����ֵ
// TODO: https://blog.csdn.net/u010462297/article/details/50469950

/* =====First Pass=====
* opengl-settings: �����ӿ�Ϊ������*
* opengl-input: ��Դ��Ϣ(���Դ��+���Դ��) & ������Ϣ(λ��+����+kd(��ɫ)) & ������λ�� & ����������
* opengl-output: 3��һάRGBͼ��ά�ȳ���=����������ֵ=3*3=9����гϵ��
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