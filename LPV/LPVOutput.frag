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
    vec3 gridSH[4];
	vec3 worldPosToMinBox = worldPos - gridMinBox;
	vec3 fGridIndex = {floor(worldPosToMinBox.x / fGridSize.x), floor(worldPosToMinBox.y / fGridSize.y), floor(worldPosToMinBox.z / fGridSize.z)};
    ivec3 iGridIndex = {int(fGridIndex.x), int(fGridIndex.y), int(fGridIndex.z)};
    vec2 vR0 = atomToVec2(imageLoad(girdTextureR0, iGridIndex).x);
    vec2 vR1 = atomToVec2(imageLoad(girdTextureR1, iGridIndex).x);
    vec2 vG0 = atomToVec2(imageLoad(girdTextureG0, iGridIndex).x);
    vec2 vG1 = atomToVec2(imageLoad(girdTextureG1, iGridIndex).x);
    vec2 vB0 = atomToVec2(imageLoad(girdTextureB0, iGridIndex).x);
    vec2 vB1 = atomToVec2(imageLoad(girdTextureB1, iGridIndex).x);
    gridSH[0] = vec3(vR0.r, vG0.r, vB0.r);
    gridSH[1] = vec3(vR0.g, vG0.g, vB0.g);
    gridSH[2] = vec3(vR1.r, vG1.r, vB1.r);
    gridSH[3] = vec3(vR1.g, vG1.g, vB1.g);

    vec3 fGridCenterIndex = fGridIndex + vec3(0.5f, 0.5f, 0.5f);
    vec3 fGridCenter = vec3(
        gridMinBox.x + fGridCenterIndex.x * fGridSize.x,
        gridMinBox.y + fGridCenterIndex.y * fGridSize.y,
        gridMinBox.z + fGridCenterIndex.z * fGridSize.z
    );

    vec3 OX = normalize(worldPos - fGridCenter);
    float thetaOX = acos(OX.z);
    float phiOX = atan(OX.y, OX.x);
    if(phiOX < 0.f) {
        phiOX += 2.f * PI;
    }
    vec3 indirect = vec3(0.f, 0.f, 0.f);
    for (int l = 0; l < 2; ++l) {
        for (int m = -l; m <= l; ++m) {
            int index = l * (l + 1) + m;
            indirect += SH(l, m, thetaOX, phiOX) * gridSH[index];
        }
    }
    indirect = vec3(max(0.f, indirect.x), max(0.f, indirect.y), max(0.f, indirect.z));

    vec3 color = direct + indirect;

	FragColor = vec4(indirect, 1.f);

    //const float gamma = 2.2;
    //FragColor.rgb = pow(FragColor.rgb, vec3(1.f / gamma));
}