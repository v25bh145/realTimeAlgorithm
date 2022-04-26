#version 330 core
layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 flux;

struct Light {
    vec3 position;
//	vec3 direction;
//	float cutOff;// 切光角，用于聚光灯(存储余弦值)
//	float outerCutOff;// 外圆锥，用于软化边缘

//  vec3 ambient;
    vec3 diffuse;
//  vec3 specular;

	// attenuation
//	float constant;
//	float linear;
//	float quadratic;
};

uniform float farPlane;
uniform Light light;

uniform sampler2D texture_diffuse1;

in VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
	vec2 fTexCoords;
} fs_in;

void main() {
	gl_FragDepth = length(light.position - fs_in.worldPos) / farPlane;
	//gl_FragDepth = length(light.position - fs_in.worldPos);

	worldPos = fs_in.worldPos;
	normal = fs_in.fNormal;

	vec3 lightDir = normalize(light.position - fs_in.worldPos);
	vec3 diff = max(dot(lightDir, fs_in.fNormal), 0.f) * light.diffuse;
	//vec3 objColor = abs(normal) * 0.8f;
	vec3 objColor = texture(texture_diffuse1, fs_in.fTexCoords).xyz;
	flux = diff * objColor;
}