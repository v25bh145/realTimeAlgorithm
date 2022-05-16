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

layout (rgba32ui, binding = 6) uniform uimage1D samplesIdxInGridTexture;

uniform samplerCube shadowWorldPosMap;
uniform samplerCube shadowFluxMap;

in vec3 fTexCoords;

// [-32767, +32767]
const float compressFactor = 5000.f;
// accuracy = 1.f / compressFactor
// fix: 使用补码表示负整数
uint compressFloatToUint16(float f) {
    int compressedNum = int(floor(f * compressFactor + 0.5f));
    uint res;
    if(compressedNum < 0) {
        res = uint(-compressedNum);
        res = ~res;
        res += 1;
    } else {
        res = uint(compressedNum);
    }
    res = res & 0xFFFF;
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

#define SH_C0 0.282094792f // 1 / 2sqrt(pi)
#define SH_C1 0.488602512f // sqrt(3/pi) / 2
vec4 evalSH_direct(vec3 dir) {	
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}

void main() {
    
	vec3 worldPos = texture(shadowWorldPosMap, fTexCoords).xyz;
	vec3 flux = texture(shadowFluxMap, fTexCoords).xyz;
    
    if(worldPos == vec3(0.f, 0.f, 0.f) || flux == vec3(0.f, 0.f, 0.f)) 
        discard;
    
    // get 3D index of VPL
    vec3 worldPosToMinBox = worldPos - gridMinBox;
    worldPosToMinBox = max(worldPosToMinBox, vec3(0.f, 0.f, 0.f));
    vec3 fGridIndex = {floor(worldPosToMinBox.x / gridSize.x), floor(worldPosToMinBox.y / gridSize.y), floor(worldPosToMinBox.z / gridSize.z)};
    ivec3 iGridIndex = {int(fGridIndex.x), int(fGridIndex.y), int(fGridIndex.z)};
    // calculate center of grid
    vec3 centerWorldPos = worldPosToMinBox + 0.5f * gridSize;
    // dir VPL to center
    vec3 VPLDir = normalize(centerWorldPos - worldPos);
    
    vec3 grid_SH[4];
    vec4 VPL_SH = evalSH_direct(VPLDir);
    grid_SH[0] = flux * VPL_SH.x;
    grid_SH[1] = flux * VPL_SH.y;
    grid_SH[2] = flux * VPL_SH.z;
    grid_SH[3] = flux * VPL_SH.w;

    /*
    float theta, phi;
    theta = acos(VPLDir.z);
    phi = acos(VPLDir.x / sin(theta));
    vec3 grid_SH[4];
    // calculate theta & phi
    for (int l = 0; l < 2; ++l) {
        for (int m = -l; m <= l; ++m) {
            int index = l * (l + 1) + m;
            grid_SH[index] = flux * SH(l, m, theta, phi);
        }
    }
    */
    
    imageAtomicAdd(girdTextureR0, iGridIndex, vec2ToAtom(vec2(grid_SH[0].r, grid_SH[1].r)));
    imageAtomicAdd(girdTextureR1, iGridIndex, vec2ToAtom(vec2(grid_SH[2].r, grid_SH[3].r)));
    imageAtomicAdd(girdTextureG0, iGridIndex, vec2ToAtom(vec2(grid_SH[0].g, grid_SH[1].g)));
    imageAtomicAdd(girdTextureG1, iGridIndex, vec2ToAtom(vec2(grid_SH[2].g, grid_SH[3].g)));
    imageAtomicAdd(girdTextureB0, iGridIndex, vec2ToAtom(vec2(grid_SH[0].b, grid_SH[1].b)));
    imageAtomicAdd(girdTextureB1, iGridIndex, vec2ToAtom(vec2(grid_SH[2].b, grid_SH[3].b)));
    
    // 最后一位为有效位(bool)
    imageStore(samplesIdxInGridTexture, 1, uvec4(1, 1, 1, 1));
}