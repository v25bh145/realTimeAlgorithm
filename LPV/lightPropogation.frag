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
// testTextureDebug 
//layout (rgba32ui, binding = 6) uniform uimage1D testTextureUint;
//layout (rgba32f, binding = 7) uniform image1D testTextureFloat;

uniform int iGridTextureSize;
uniform vec3 fGridSize;
uniform vec3 gridMinBox;

in flat int iGridIndex;

#define SH_C0 0.282094792f // 1 / 2sqrt(pi)
#define SH_C1 0.488602512f // sqrt(3/pi) / 2
vec4 evalSH_direct(vec3 dir) {	
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}
// [-32767, +32767]
const float PI = acos(-1.f);
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
vec3 storeGridInputSH[4];
void storeGrid(ivec3 grid) {
    imageAtomicAdd(girdTextureR0, grid, vec2ToAtom(vec2(storeGridInputSH[0].r, storeGridInputSH[1].r)));
    imageAtomicAdd(girdTextureR1, grid, vec2ToAtom(vec2(storeGridInputSH[2].r, storeGridInputSH[3].r)));
    imageAtomicAdd(girdTextureG0, grid, vec2ToAtom(vec2(storeGridInputSH[0].g, storeGridInputSH[1].g)));
    imageAtomicAdd(girdTextureG1, grid, vec2ToAtom(vec2(storeGridInputSH[2].g, storeGridInputSH[3].g)));
    imageAtomicAdd(girdTextureB0, grid, vec2ToAtom(vec2(storeGridInputSH[0].b, storeGridInputSH[1].b)));
    imageAtomicAdd(girdTextureB1, grid, vec2ToAtom(vec2(storeGridInputSH[2].b, storeGridInputSH[3].b)));
}
vec3 loadGridOutputSH[4];
void loadGrid(ivec3 grid) {
    vec2 vR0 = atomToVec2(imageLoad(girdTextureR0, grid).x);
    vec2 vR1 = atomToVec2(imageLoad(girdTextureR1, grid).x);
    vec2 vG0 = atomToVec2(imageLoad(girdTextureG0, grid).x);
    vec2 vG1 = atomToVec2(imageLoad(girdTextureG1, grid).x);
    vec2 vB0 = atomToVec2(imageLoad(girdTextureB0, grid).x);
    vec2 vB1 = atomToVec2(imageLoad(girdTextureB1, grid).x);
    loadGridOutputSH[0] = vec3(vR0.r, vG0.r, vB0.r);
    loadGridOutputSH[1] = vec3(vR0.g, vG0.g, vB0.g);
    loadGridOutputSH[2] = vec3(vR1.r, vG1.r, vB1.r);
    loadGridOutputSH[3] = vec3(vR1.g, vG1.g, vB1.g);
}
bool gridOutOfRange(ivec3 nowGrid) {
    if(0 <= nowGrid.x && nowGrid.x < iGridTextureSize
    && 0 <= nowGrid.y && nowGrid.y < iGridTextureSize
    && 0 <= nowGrid.z && nowGrid.z < iGridTextureSize)
        return false;
    return true;
}
vec3 getGridCenter(ivec3 grid) {
    vec3 fGrid = vec3(grid) + vec3(0.5f, 0.5f, 0.5f);
    vec3 res = vec3(
        gridMinBox.x + fGrid.x * fGridSize.x,
        gridMinBox.y + fGrid.y * fGridSize.y,
        gridMinBox.z + fGrid.z * fGridSize.z
    );
    return res;
}

// right=0.400562f
// 4*front back up down = 0.423698f
// 0 front 1 right 2 up 3 back 4 left 5 down

// 从nowGrid开始向其他5/6面的格子传输
void propogateOnce(ivec3 nowGrid) {
    loadGrid(nowGrid);
    vec3 girdNow_SH[4] = loadGridOutputSH;
    vec3 nowGridCenter = getGridCenter(nowGrid); // O point

    ivec3 deltaOfSideTable[6] = {
        ivec3(1, 0, 0),
        ivec3(0, 1, 0),
        ivec3(0, 0, 1),
        ivec3(-1, 0, 0),
        ivec3(0, -1, 0),
        ivec3(0, 0, -1),
    };
    int oppoSideTable[6] = {3, 4, 5, 0, 1, 2};
    ivec4 nearSideTable[6] = {
        ivec4(1, 2, 4, 5),
        ivec4(0, 2, 3, 5),
        ivec4(0, 1, 3, 4),
        ivec4(1, 2, 4, 5),
        ivec4(0, 2, 3, 5),
        ivec4(0, 1, 3, 4),
    };
    // 向5个格子传输
    for(int i = 0; i < 6; i++) {
        ivec3 nextGrid = nowGrid + deltaOfSideTable[i];
        /*
        ivec3 originToNow = nowGrid - originGrid;
        ivec3 originToNext = nextGrid - originGrid;
        if(abs(originToNext.x) < abs(originToNow.x)) continue;
        if(abs(originToNext.y) < abs(originToNow.y)) continue;
        if(abs(originToNext.z) < abs(originToNow.z)) continue;
        */
        if(gridOutOfRange(nextGrid)) continue;

        vec3 nextGridCenter = getGridCenter(nextGrid); // A point

        vec3 girdNext_SH[4] = {
            vec3(0.f, 0.f, 0.f),
            vec3(0.f, 0.f, 0.f),
            vec3(0.f, 0.f, 0.f),
            vec3(0.f, 0.f, 0.f)
        };
        // 向1个格子的5个面传输
        for(int j = 0; j < 6; j++) {
            if(j == i) continue;
            vec3 LiOX;
            vec3 edgeCenter = vec3(
                nextGridCenter.x + float(deltaOfSideTable[j].x) * fGridSize.x / 2.f,
                nextGridCenter.y + float(deltaOfSideTable[j].y) * fGridSize.y / 2.f,
                nextGridCenter.z + float(deltaOfSideTable[j].z) * fGridSize.z / 2.f
            ); // X point
            vec3 OX = normalize(edgeCenter - nowGridCenter);
            float thetaOX = acos(OX.z);
            float phiOX = atan(OX.y, OX.x);
            if(phiOX < 0.f) {
                phiOX += 2.f * PI;
            }
            vec3 XO = -OX;

            vec4 OX_SH = evalSH_direct(OX);
            LiOX += OX_SH.x * girdNow_SH[0];
            LiOX += OX_SH.y * girdNow_SH[1];
            LiOX += OX_SH.z * girdNow_SH[2];
            LiOX += OX_SH.w * girdNow_SH[3];

            vec3 Nx = normalize(nextGridCenter - edgeCenter);
            float cosAngle = abs(dot(Nx, XO));
            vec3 LiXA = LiOX * cosAngle / PI;
            vec3 LEdgeToA = LiXA;
            if(j == oppoSideTable[i]) {
                LEdgeToA *= 0.400562f;
            } else {
                LEdgeToA *= 0.423698f;
            }

            vec3 AX = -Nx;
            float thetaAX = acos(AX.z);
            float phiAX = atan(AX.y, AX.x);
            if(phiAX < 0.f) {
                phiAX += 2.f * PI;
            }
            // calculate theta & phi
            vec4 SH_AX = evalSH_direct(AX);
            girdNext_SH[0] = SH_AX.x * LEdgeToA;
            girdNext_SH[1] = SH_AX.y * LEdgeToA;
            girdNext_SH[2] = SH_AX.z * LEdgeToA;
            girdNext_SH[3] = SH_AX.w * LEdgeToA;
        }
        storeGridInputSH = girdNext_SH;
        storeGrid(nextGrid);
        //propogate(nextGrid, oppoSideTable[i], count + 1);
    }
}

void main() {
    int iGridTextureSize2 = iGridTextureSize * iGridTextureSize;
    int gridXIndex = iGridIndex / iGridTextureSize2;
    int gridIndexYZ = iGridIndex - gridXIndex * iGridTextureSize2;
    int gridYIndex = gridIndexYZ / iGridTextureSize;
    int gridIndexZ = gridIndexYZ - gridYIndex * iGridTextureSize;
    int gridZIndex = gridIndexZ;
    ivec3 gridIndexVec = ivec3(gridXIndex, gridYIndex, gridZIndex);
    propogateOnce(gridIndexVec);
    // testTextureDebug 
    //imageStore(testTextureUint, iGridIndex, uvec4(gridIndexVec, 1));
    //imageStore(testTextureFloat, iGridIndex, vec4(float(iGridIndex), 1.f, 1.f, 1.f));
}