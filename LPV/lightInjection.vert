#version 420 core
layout(location = 0) in vec3 sampleUv;
layout(location = 1) in float sampleIndex;

uniform samplerCube shadowWorldPosMap;
uniform samplerCube shadowFluxMap;

out VS_OUT {
	vec3 worldPos;
	vec3 flux;
	vec3 uv;
	flat int sampleIndex;
} vs_out;

void main() {
	vs_out.worldPos = texture(shadowWorldPosMap, sampleUv).xyz;
	vs_out.flux = texture(shadowFluxMap, sampleUv).xyz;
	vs_out.uv = sampleUv;
	vs_out.sampleIndex = int(floor(sampleIndex + 0.5f));
}