#version 420 core
// girdTextureR: c1 ~ c4
layout (r32ui, binding = 0) uniform uimage3D girdTextureR0;
layout (r32ui, binding = 1) uniform uimage3D girdTextureR1;
// girdTextureG: c1 ~ c4
layout (r32ui, binding = 2) uniform uimage3D girdTextureG0;
layout (r32ui, binding = 3) uniform uimage3D girdTextureG1;
// girdTextureB: c1 ~ c4
layout (r32ui, binding = 4) uniform uimage3D girdTextureB0;
layout (r32ui, binding = 5) uniform uimage3D girdTextureB1;

layout (rgba32ui, binding = 6) uniform uimage1D testTextureUint;
layout (rgba32f, binding = 7) uniform image1D testTextureFloat;

in VS_OUT {
    flat ivec3 gridIndex;
    flat int sampleIndex;
} fs_in;

float P(float x, int l, int m) {
    if (m < 0 || l < 0) { 
        return 0.f; 
    }
    // P^m_m
    float pmm = 1.f;
    if (m > 0) {
        // somx2 = (1-x^2)^{1/2}
        float somx2 = sqrt((1.f + x) * (1.f - x));
        // fact: 双阶乘函数，且2m-1为奇数，可解n!!=n(n-2)……×3×1
        float fact = 1.f;
        for (int i = 0; i < m; ++i) {
            pmm *= -1.f * fact * somx2;
            fact += 2.f;
        }
    }
    // (1)式
    if (m == l) return pmm;
    // P^m_{m+1}
    float pmm1 = x * (2.f * float(m) + 1.f) * pmm;
    // (2)式
    if (m + 1 == l) return pmm1;
    // 将p^m_m和P^m_{m+1}底下的m与m+1 逐层向上推导，直到得到p^m_{l-2}和P^m_{l-1}
    float pml = 0.f;
    for (int ll = m + 2; ll <= l; ++ll) {
        pml = x * (2.f * float(ll) - 1.f) * pmm1 - (float(ll) + float(m) - 1.f) * pmm;
        pml /= float(ll) - float(m);
        pmm = pmm1;
        pmm1 = pml;
    }
    return pml;
}
float K(int l, int m) {
    const float PI = acos(-1);
    m = abs(m);
    int fact1 = 1, fact2 = 1;
    for (int i = 1; i <= l + m; ++i) fact1 *= i;
    for (int i = 1; i <= l - m; ++i) fact2 *= i;
    float tmp = (2.f * float(l) + 1.f) * float(fact2) / (4.f * PI * float(fact1));
    return sqrt(tmp);
}
float SH(int l, int m, float theta, float phi) {
    const float PI = acos(-1.f);
    if (m < -l || m > l || l < 0 || theta < 0.f || theta > PI || phi < 0.f || phi > 2 * PI) { 
        return 0.f; 
    }
    const float sqrt2 = sqrt(2.f);
    if (m == 0) return K(l, 0) * P(cos(theta), l, 0);
    else if (m > 0) return sqrt2 * K(l, m) * cos(m * phi) * P(cos(theta), l, m);
    else return sqrt2 * K(l, m) * sin(-m * phi) * P(cos(theta), l, -m);
}
// [-32767, +32767]
const float compressFactor = 5000.f;
// accuracy = 1.f / compressFactor
// fix: 使用补码表示负整数
float deCompressUint16ToFloat(uint u) {
    float res;
    int compressedNum;
    if(((u >> 15) & 0x1) == 1) {
        u -= 1;
        u = ~u;
        u = u & 0xFFFF;
        compressedNum = -int(u);
    } else {
        compressedNum = int(u);
    }
    res = float(compressedNum) / compressFactor;
    return res;
}
vec2 atomToVec2(uint atom)
{
    vec2 res;
    res.r = deCompressUint16ToFloat(atom & 0xFFFF);
    res.g = deCompressUint16ToFloat((atom >> 16) & 0xFFFF);
    return res;
}

void main() {
    uvec4 res = imageLoad(girdTextureR0, fs_in.gridIndex);
    uint uRes = uint(res.x);
    vec2 vRes = atomToVec2(uRes);

    uint test = 0xFF7D;
    imageStore(testTextureUint, fs_in.sampleIndex, uvec4(res.x, test, (test >> 15), 1));
    imageStore(testTextureFloat, fs_in.sampleIndex, vec4(vRes, deCompressUint16ToFloat(0xFF7D), 1.f));
}