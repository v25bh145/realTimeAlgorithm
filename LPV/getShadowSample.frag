#version 420 core
layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 flux;

struct Light {
	vec3 position;
	vec3 diffuse;
};
uniform Light light;
uniform float farPlane;

in VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
} fs_in;

void main() {
	gl_FragDepth = length(light.position - fs_in.worldPos) / farPlane;

	worldPos = fs_in.worldPos;
	// vec3 normal = fs_in.fNormal;

	vec3 lightDir = normalize(light.position - fs_in.worldPos);
	vec3 diff = max(dot(lightDir, fs_in.fNormal), 0.f) * light.diffuse;
	vec3 objColor = abs(fs_in.fNormal) * 0.8f;
	flux = diff * objColor;
}