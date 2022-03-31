#ifndef SHALGORITHM_H
#define SHALGORITHM_H
#include "shDefault.h"
#include <vector>

double P(double x, int l, int m);
double K(int l, int m);
double SH(int l, int m, double theta, double phi);

#define n_bands 3
#define n_coeff n_bands * n_bands
struct SHSample {
    glm::vec3 sph;
    glm::vec3 vec;
    double* coeff;
};
void SH_setup_spherical_samples(vector<SHSample>& samples, int sqrt_n_samples);
// ����һ������{dx, dy, dz}������������ӦIBL�ϵ��RGB��ɫ
typedef glm::vec3(*SH_vector_fn_rgb)(SHSample sample);
vector<glm::vec3> SH_project_vector_function_rgb(SH_vector_fn_rgb fn, vector<SHSample> sh_samples);

// TODO:
// 1. ����dir��ȡ������������ͼ�ڵ���ɫ�������(SH_vector_fn_rgb)
// 2. �ó�ͶӰϵ��������ͶӰϵ����OpenGL��Ⱦ�����

// ����cube�����Ĳ���
void set_sh_light_from_cube_paraments(vector<unsigned char*> input_datas, int input_textureSize, int input_nrComponents);
// cube�������㷨
glm::vec3 gen_sh_light_from_cube(SHSample sample);
#endif