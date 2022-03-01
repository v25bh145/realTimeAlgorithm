#version 330 core
struct Light {
    vec3 position;
//	vec3 direction;
//	float cutOff;// 切光角，用于聚光灯(存储余弦值)
//	float outerCutOff;// 外圆锥，用于软化边缘

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	// attenuation
	float constant;
	float linear;
	float quadratic;
};

out vec4 FragColor;

uniform samplerCube depthMap;
uniform samplerCube worldPosMap;
uniform samplerCube normalMap;
uniform samplerCube fluxMap;
uniform sampler1D randomMap;
uniform float farPlane;
uniform Light light;
uniform vec3 viewPos;

const float PI = 3.14159265359;
const float MAX_THETA = 1.570796326795;
const float MAX_PHI = 6.28318530718;
const float gamma = 2.2;

float defaultShadowLight = 0.05f;
float shadowBias = 0.05f;
float sampleThetaRange = PI / 5.f;
float samplePhiRange = PI / 5.f;
int sampleNum = 50;
int shininess = 32;
float shadowOffset = 0.1f;
int shadowSamples = 10;

in VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
} fs_in;

void main() {
	vec3 fragToLight = fs_in.worldPos - light.position;


	vec3 fragToLightDir = normalize(fragToLight);

	float currentDepth = length(fragToLight);
	// hard shadow
	//float closestDepth = texture(depthMap, fragToLight).r;
	//closestDepth *= farPlane;
	//float shadow = currentDepth - shadowBias > closestDepth ? defaultShadowLight : 1.f;
	// soft shadow
	float shadow = 0.f;
	for(float x = -shadowOffset; x < shadowOffset; x += 2.f * shadowOffset / shadowSamples){
		for(float y = -shadowOffset; y < shadowOffset; y += 2.f * shadowOffset / shadowSamples) {
            for(float z = -shadowOffset; z < shadowOffset; z += 2.f * shadowOffset / shadowSamples){
                float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; 
                closestDepth *= farPlane;   // Undo mapping [0;1]
                if(currentDepth - shadowBias > closestDepth)
                    shadow += 1.f;
				else
					shadow += defaultShadowLight;
            }
		}
	}
	shadow /= shadowSamples * shadowSamples * shadowSamples;

	// simple way to generate color
	vec3 objColor = abs(fs_in.fNormal) * 0.8f;

	float cosTheta = fragToLightDir.z;
	float sinTheta = sqrt(1 - cosTheta * cosTheta);
	float cosPhi = fragToLightDir.x / sinTheta;
	float sinPhi = fragToLightDir.y / sinTheta;
	vec3 indirect = vec3(0.f, 0.f, 0.f);
	for(int i = 0; i < sampleNum; ++i) {
		// (-1, -1) TO (1, 1)
		vec2 randomBias = (texelFetch(randomMap, i, 0).rg - 0.5f) * 2.f;
		float thetaDelta = sampleThetaRange * randomBias.r;
		float phiDelta = samplePhiRange * randomBias.g;

		float cosThetaDelta = cos(thetaDelta);
		float sinThetaDelta = sin(thetaDelta);
		float cosPhiDelta = cos(phiDelta);
		float sinPhiDelta = sin(phiDelta);

		float cosThetaBias = cosTheta * cosThetaDelta - sinTheta * sinThetaDelta;
		float sinThetaBias = sinTheta * cosThetaDelta + cosTheta * sinThetaDelta;
		float cosPhiBias = sinPhi * cosPhiDelta + cosPhi * sinPhiDelta;
		float sinPhiBias = cosPhi * cosPhiDelta - sinPhi * sinPhiDelta;

		vec3 dirBias = vec3(sinThetaBias * cosPhiBias, sinThetaBias * sinPhiBias, cosThetaBias);

		vec3 indirectFlux = texture(fluxMap, dirBias).rgb;
		vec3 worldPosBias = texture(worldPosMap, dirBias).xyz;
		vec3 normalBias = texture(normalMap, dirBias).xyz;

		vec3 minusBias = worldPosBias - fs_in.worldPos;
		float BiasEmit = max(0, dot(normalBias, -minusBias));
		float BiasRecv = max(0, dot(fs_in.fNormal, minusBias));
		float attenuate = pow(length(minusBias), 2);
		indirect += indirectFlux * BiasEmit * BiasRecv / attenuate;
	}
	indirect /= sampleNum;

	vec3 viewDir = viewPos - fs_in.worldPos;
	vec3 lightDir = -fragToLightDir;

	vec3 diff = max(dot(lightDir, fs_in.fNormal), 0.f) * light.diffuse;
	vec3 diffuse = diff * light.diffuse;

	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(0, dot(halfwayDir, fs_in.fNormal)), shininess);
	vec3 specular = spec * light.specular;

	vec3 direct = ((diffuse + specular) * (1.f - shadow) + light.ambient) * objColor;

	FragColor = vec4(direct + indirect, 1.f);
	
	FragColor.rgb = pow(FragColor.rgb, vec3(1.f / gamma));
}