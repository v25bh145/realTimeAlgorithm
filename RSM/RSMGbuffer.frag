#version 330 core
layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 flux;

struct Light {
    vec3 position;
//	vec3 direction;
//	float cutOff;// �й�ǣ����ھ۹��(�洢����ֵ)
//	float outerCutOff;// ��Բ׶����������Ե

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

in VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
} fs_in;

void main() {
	gl_FragDepth = length(light.position - fs_in.worldPos) / farPlane;

	worldPos = fs_in.worldPos;
	normal = fs_in.fNormal;

	vec3 lightDir = normalize(light.position - fs_in.worldPos);
	vec3 diff = max(dot(lightDir, fs_in.fNormal), 0.f) * light.diffuse;
	vec3 objColor = abs(normal) * 0.8f;
	flux = diff * objColor;
}