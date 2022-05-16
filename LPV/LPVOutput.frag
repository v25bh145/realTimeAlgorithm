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

in vec2 fTexCoords;

struct Light {
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
};
uniform Light light;
uniform float farPlane;

uniform samplerCube shadowWorldPosMap;
uniform sampler2D screenWorldPosMap;
uniform sampler2D screenDiffuseMap;
uniform sampler2D screenNormalMap;

uniform uint iGridTextureSize;
uniform vec3 fGridSize;
uniform vec3 gridMinBox;

uniform vec3 viewPos;

out vec4 FragColor;

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
vec3 calculateIndirect(ivec3 iGridIndex, vec3 worldPos) {
	//vec3 worldPosToMinBox = worldPos - gridMinBox;
    //worldPosToMinBox = max(worldPosToMinBox, vec3(0.f, 0.f, 0.f));
	//vec3 fGridIndex = {floor(worldPosToMinBox.x / fGridSize.x), floor(worldPosToMinBox.y / fGridSize.y), floor(worldPosToMinBox.z / fGridSize.z)};
    //ivec3 iGridIndex = {int(fGridIndex.x), int(fGridIndex.y), int(fGridIndex.z)};
	vec3 fGridIndex = vec3(iGridIndex);
    vec2 vR0 = atomToVec2(imageLoad(girdTextureR0, iGridIndex).x);
    vec2 vR1 = atomToVec2(imageLoad(girdTextureR1, iGridIndex).x);
    vec2 vG0 = atomToVec2(imageLoad(girdTextureG0, iGridIndex).x);
    vec2 vG1 = atomToVec2(imageLoad(girdTextureG1, iGridIndex).x);
    vec2 vB0 = atomToVec2(imageLoad(girdTextureB0, iGridIndex).x);
    vec2 vB1 = atomToVec2(imageLoad(girdTextureB1, iGridIndex).x);
    vec3 gridSH[4];
    gridSH[0] = vec3(vR0.r, vG0.r, vB0.r);
    gridSH[1] = vec3(vR0.g, vG0.g, vB0.g);
    gridSH[2] = vec3(vR1.r, vG1.r, vB1.r);
    gridSH[3] = vec3(vR1.g, vG1.g, vB1.g);

    vec3 fGridCenter = vec3(fGridIndex.x * fGridSize.x + gridMinBox.x, fGridIndex.y * fGridSize.y + gridMinBox.y, fGridIndex.z * fGridSize.z + gridMinBox.z) + 0.5f * fGridSize;

    vec3 OX = normalize(worldPos - fGridCenter);
    vec3 indirect = vec3(0.f, 0.f, 0.f);

    vec4 VPL_SH = evalSH_direct(OX);
    indirect += gridSH[0] * VPL_SH.x;
    indirect += gridSH[1] * VPL_SH.y;
    indirect += gridSH[2] * VPL_SH.z;
    indirect += gridSH[3] * VPL_SH.w;

    indirect = abs(indirect);
    indirect = vec3(min(1.f, indirect.x), min(1.f, indirect.y), min(1.f, indirect.z));
    return indirect;
}
void main() {
	vec3 worldPos = texture(screenWorldPosMap, fTexCoords).xyz;
	vec3 kd = texture(screenDiffuseMap, fTexCoords).xyz;
	vec3 normal = texture(screenNormalMap, fTexCoords).xyz;

	vec3 fragToLight = worldPos - light.position;
	vec3 fragToLightDir = normalize(fragToLight);
	float currentDepth = length(fragToLight);

	// shadow
	float defaultShadowLight = 0.05f;
	float shadowBias = 0.08f;
	int shadowSamples = 10;
	float shadowOffset = 0.1f;
	float shadowValue = 0.f;
	for(float x = -shadowOffset; x < shadowOffset; x += 2.f * shadowOffset / shadowSamples){
		for(float y = -shadowOffset; y < shadowOffset; y += 2.f * shadowOffset / shadowSamples) {
            for(float z = -shadowOffset; z < shadowOffset; z += 2.f * shadowOffset / shadowSamples){
				vec3 closestWorldPos = texture(shadowWorldPosMap, fragToLight + vec3(x, y, z)).xyz; 
                float closestDepth = length(closestWorldPos - light.position);
                if(currentDepth - shadowBias > closestDepth)
                    shadowValue += 1.f;
				else
					shadowValue += defaultShadowLight;
            }
		}
	}
	shadowValue /= shadowSamples * shadowSamples * shadowSamples;

	// direct
	vec3 viewDir = viewPos - worldPos;
	vec3 lightDir = -fragToLightDir;

	vec3 diff = max(dot(lightDir, normal), 0.f) * light.diffuse;
	vec3 diffuse = diff * light.diffuse;

	int shininess = 32;
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(0, dot(halfwayDir, normal)), shininess);
	vec3 specular = spec * light.specular;

	vec3 direct = ((diffuse + specular) * (1.f - shadowValue) + light.ambient) * kd;

	// indirect
	vec3 worldPosToMinBox = worldPos - gridMinBox;
    worldPosToMinBox = max(worldPosToMinBox, vec3(0.f, 0.f, 0.f));
	vec3 fGridIndex = {floor(worldPosToMinBox.x / fGridSize.x), floor(worldPosToMinBox.y / fGridSize.y), floor(worldPosToMinBox.z / fGridSize.z)};
    ivec3 iGridIndex = {int(fGridIndex.x), int(fGridIndex.y), int(fGridIndex.z)};
    
    vec3 indirect = vec3(0.f, 0.f, 0.f);
    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            for(int k = -1; k <= 1; k++) {
                indirect += calculateIndirect(ivec3(iGridIndex.x + i, iGridIndex.y + j, iGridIndex.z + k), worldPos);
            }
        }
    }
    indirect = indirect / 9.f;
    
    //vec3 indirect = calculateIndirect(iGridIndex, worldPos);

    vec3 color = direct + indirect;

	FragColor = vec4(color, 1.f);

    const float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.f / gamma));
}