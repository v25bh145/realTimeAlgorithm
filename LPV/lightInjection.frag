#version 420 core
// 见下博客，将四通道压缩存储为一通道，因为imageAtomicAdd只支持写入uint型数据
// https://blog.csdn.net/u010462297/article/details/50469950
// 4通道，每个通道8位，将球谐系数扩大了10000倍以整数形式保存

// girdTextureR: c1 ~ c4
layout (r32ui, binding = 0) uniform uimage3D girdTextureR0;
layout (r32ui, binding = 1) uniform uimage3D girdTextureR1;
// girdTextureG: c1 ~ c4
layout (r32ui, binding = 2) uniform uimage3D girdTextureG0;
layout (r32ui, binding = 3) uniform uimage3D girdTextureG1;
// girdTextureB: c1 ~ c4
layout (r32ui, binding = 4) uniform uimage3D girdTextureB0;
layout (r32ui, binding = 5) uniform uimage3D girdTextureB1;

layout (rgba32f, binding = 6) uniform image1D testTexture;

in VS_OUT {
	vec3 worldPos;
	vec3 flux;
	vec3 uv;
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
uint compressFloatToUint16(float f) {
    uint res;
    if(f > 0.f) {
        res = uint(abs(f) * compressFactor);
    } else {
        res = (1 << 15) + uint(abs(f) * compressFactor);
    }
    return res;
}
uint vec2ToAtom(vec2 fColor)
{
    uvec2 uColor;
    uColor.r = compressFloatToUint16(fColor.r);
    uColor.g = compressFloatToUint16(fColor.g);
    return uColor.r | (uColor.g << 16);
}

uniform vec3 gridSize;
uniform vec3 gridMinBox;

void main() {
    if(fs_in.worldPos == vec3(0.f, 0.f, 0.f)) 
        discard;
    // get 3D index of VPL
    vec3 worldPosToMinBox = fs_in.worldPos - gridMinBox;
    vec3 fGridIndex = {floor(worldPosToMinBox.x / gridSize.x), floor(worldPosToMinBox.y / gridSize.y), floor(worldPosToMinBox.z / gridSize.z)};
    ivec3 iGridIndex = {int(fGridIndex.x), int(fGridIndex.y), int(fGridIndex.z)};
    // calculate center of grid
    vec3 centerWorldPos = {(float(iGridIndex.x) + 0.5f) * gridSize.x + gridMinBox.x, (float(iGridIndex.y) + 0.5f) * gridSize.y + gridMinBox.y, (float(iGridIndex.z) + 0.5f) * gridSize.z + gridMinBox.z};
    // dir VPL to center
    vec3 VPLDir = normalize(centerWorldPos - fs_in.worldPos);
    
	float VPL_SH[4];
    float theta, phi;
    theta = acos(VPLDir.z);
    phi = acos(VPLDir.x / sin(theta));
    // calculate theta & phi
    for (int l = 0; l < 2; ++l) {
        for (int m = -l; m <= l; ++m) {
            int index = l * (l + 1) + m;
            VPL_SH[index] = SH(l, m, theta, phi);
        }
    }
    vec3 gird_SH[4];
    for(int i = 0; i < 2 * 2; i++) {
        gird_SH[i] = fs_in.flux * VPL_SH[i];
    }
    imageAtomicAdd(girdTextureR0, iGridIndex, vec2ToAtom(vec2(gird_SH[0].r, gird_SH[1].r)));
    imageAtomicAdd(girdTextureR1, iGridIndex, vec2ToAtom(vec2(gird_SH[2].r, gird_SH[3].r)));
    imageAtomicAdd(girdTextureG0, iGridIndex, vec2ToAtom(vec2(gird_SH[0].g, gird_SH[1].g)));
    imageAtomicAdd(girdTextureG1, iGridIndex, vec2ToAtom(vec2(gird_SH[2].g, gird_SH[3].g)));
    imageAtomicAdd(girdTextureB0, iGridIndex, vec2ToAtom(vec2(gird_SH[0].b, gird_SH[1].b)));
    imageAtomicAdd(girdTextureB1, iGridIndex, vec2ToAtom(vec2(gird_SH[2].b, gird_SH[3].b)));
    
    //imageStore(testTexture, 0, vec4(gird_SH[0].r, gird_SH[1].r, gird_SH[2].r, gird_SH[3].r));
    imageStore(testTexture, fs_in.sampleIndex, vec4(fGridIndex, gird_SH[0].r));
}