#include "shAlgorithm.h"
double P(double x, int l, int m) {
    if (m < 0 || l < 0) { cout << "l&m should > 0" << endl; return 0.f; }
    // P^m_m
    double pmm = 1.f;
    if (m > 0) {
        // somx2 = (1-x^2)^{1/2}
        double somx2 = sqrt((1.f + x) * (1.f - x));
        // fact: 双阶乘函数，且2m-1为奇数，可解n!!=n(n-2)……×3×1
        double fact = 1.f;
        for (int i = 0; i < m; ++i) {
            pmm *= -1.f * fact * somx2;
            fact += 2.f;
        }
    }
    // (1)式
    if (m == l) return pmm;
    // P^m_{m+1}
    double pmm1 = x * (2.f * double(m) + 1.f) * pmm;
    // (2)式
    if (m + 1 == l) return pmm1;
    // 将p^m_m和P^m_{m+1}底下的m与m+1 逐层向上推导，直到得到p^m_{l-2}和P^m_{l-1}
    double pml = 0.f;
    for (int ll = m + 2; ll <= l; ++ll) {
        pml = x * (2.f * double(ll) - 1.f) * pmm1 - (double(ll) + double(m) - 1.f) * pmm;
        pml /= double(ll) - double(m);
        pmm = pmm1;
        pmm1 = pml;
    }
    return pml;
}
double K(int l, int m) {
    const double PI = acos(-1);
    m = abs(m);
    int fact1 = 1, fact2 = 1;
    for (int i = 1; i <= l + m; ++i) fact1 *= i;
    for (int i = 1; i <= l - m; ++i) fact2 *= i;
    double tmp = (2.f * double(l) + 1.f) * double(fact2) / (4.f * PI * double(fact1));
    return sqrt(tmp);
}
double SH(int l, int m, double theta, double phi) {
    const double PI = acos(-1.f);
    if (m < -l || m > l || l < 0 || theta < 0.f || theta > PI || phi < 0.f || phi > 2 * PI) { cout << "Error: SH() range" << endl; return 0.f; }
    const double sqrt2 = sqrt(2.f);
    if (m == 0) return K(l, 0) * P(cos(theta), l, 0);
    else if (m > 0) return sqrt2 * K(l, m) * cos(m * phi) * P(cos(theta), l, m);
    else return sqrt2 * K(l, m) * sin(-m * phi) * P(cos(theta), l, -m);
}
double random() {
    const double maxNum = 1e4;
    double res = (rand() % int(maxNum)) / maxNum;
    return res;
}
void SH_setup_spherical_samples(vector<SHSample>& samples, int sqrt_n_samples) {
    const double PI = acos(-1.f);
    double oneOverN = 1.f / sqrt_n_samples;
    const double EPSILON = 1e-6;
    for (int a = 0.f; a < sqrt_n_samples; ++a) {
        for (int b = 0.f; b < sqrt_n_samples; ++b) {
            double xi1 = random();
            double xi2 = random();
            double sqrt1_xi2_2 = 2.f * sqrt(xi1 * (1.f - xi1));
            double PI_xi2_2 = 2.f * PI * xi2;
            double x = cos(PI_xi2_2) * sqrt1_xi2_2;
            double y = sin(PI_xi2_2) * sqrt1_xi2_2;
            double z = 1.f - 2.f * xi1;
            double theta = acos(1.f - 2.f * xi1);
            double phi = 2.f * PI * xi2;

            SHSample sample;
            sample.vec = { x, y, z };
            sample.sph = { theta, phi, 1.f };
            sample.coeff = new double[n_coeff];

            for (int l = 0; l < n_bands; ++l) {
                for (int m = -l; m <= l; ++m) {
                    int index = l * (l + 1) + m;
                    sample.coeff[index] = SH(l, m, theta, phi);
                }
            }
            samples.push_back(sample);
        }
    }
}
vector<glm::vec3> SH_project_vector_function_rgb(SH_vector_fn_rgb fn, vector<SHSample> sh_samples) {
    const double PI = acos(-1.f);
    const double area = 4.f * PI;
    vector<glm::vec3> result;
    for (int n = 0; n < n_coeff; ++n) {
        result.push_back({ 0.f, 0.f, 0.f });
    }
    for (const SHSample& sample : sh_samples) {
        glm::vec3 color = fn(sample);
        for (int n = 0; n < n_coeff; ++n) {
            result[n] += (color * float(sample.coeff[n]));
        }
    }
    float factor = area / sh_samples.size();
    for_each(result.begin(), result.end(), [factor](glm::vec3& dir) {
        dir = dir * factor;
    });
    return result;
}
static vector<unsigned char*> datas; int textureSize, nrComponents;
void set_sh_light_from_cube_paraments(vector<unsigned char*> input_datas, int input_textureSize, int input_nrComponents) {
    datas = input_datas;
    textureSize = input_textureSize;
    nrComponents = input_nrComponents;
}
// 立方贴图各个2D贴图的纹理坐标见：https://blog.csdn.net/qjh5606/article/details/89847297
glm::vec3 gen_sh_light_from_cube(SHSample sample)
{
    const double PI = acos(-1.f);
    const double EPSILON = 1e-6;
    const double INF = 1e7;
    double width = double(textureSize);
    double u, v;
    unsigned char* data;

    // 类似于包围盒-光线求交算法
    double x = sample.vec.x, y = sample.vec.y, z = sample.vec.z;
    assert(abs(x * x + y * y + z * z - 1) < EPSILON);
    double w_over_2 = width / 2.f;
    double xAxis = x >= 0.f ? w_over_2 : -w_over_2;
    double yAxis = y >= 0.f ? w_over_2 : -w_over_2;
    double zAxis = z >= 0.f ? w_over_2 : -w_over_2;
    double tx = (abs(x) < EPSILON ? INF : xAxis / x), ty = (abs(y) < EPSILON ? INF : yAxis / y), tz = (abs(z) < EPSILON ? INF : zAxis / z);
    if (tx <= ty && tx <= tz) {
        if (xAxis > 0.f) {
            // 0 +x
            u = w_over_2 - z * tx;
            v = w_over_2 - y * tx;
            data = datas[0];
        }
        else {
            // 1 -x
            u = w_over_2 + z * tx;
            v = w_over_2 - y * tx;
            data = datas[1];
        }
    }
    else if (ty <= tx && ty <= tz) {
        glm::vec2 xz = { x * ty, z * ty };
        if (yAxis > 0.f) {
            // 2 +y
            u = w_over_2 + x * ty;
            v = w_over_2 + z * ty;
            data = datas[2];
        }
        else {
            // 3 -y
            u = w_over_2 + x * ty;
            v = w_over_2 - z * ty;
            data = datas[3];
        }
    }
    else {
        glm::vec2 xy = { x * tz,  y * tz };
        if (zAxis > 0.f) {
            // 4 +z
            u = w_over_2 + x * tz;
            v = w_over_2 - y * tz;
            data = datas[4];
        }
        else {
            // 5 -z
            u = w_over_2 - x * tz;
            v = w_over_2 - y * tz;
            data = datas[5];
        }
    }

    // get colors from data
    glm::vec3 color = glm::vec3(1.f);
    int index = (int(u) * width + int(v)) * nrComponents;
    if (nrComponents == 1)
        color = { (float)data[index], 1.f, 1.f};
    else if (nrComponents == 3)
        color = { (float)data[index], (float)data[index + 1], (float)data[index + 2]};
    else if (nrComponents == 4)
        color = { (float)data[index], (float)data[index + 1], (float)data[index + 2]};
    color /= 255.f;
    return color;
}
