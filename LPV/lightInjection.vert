#version 420 core
layout(location = 0) in vec3 sampleUv;
//layout(location = 1) in float sampleIndex;

uniform samplerCube worldPosMap;
uniform samplerCube fluxMap;

out VS_OUT {
	vec3 worldPos;
	vec3 flux;
	vec3 uv;
	//uint sampleIndex;
} vs_out;

void main() {
	vs_out.worldPos = texture(worldPosMap, sampleUv).xyz;
	vs_out.flux = texture(fluxMap, sampleUv).xyz;
	vs_out.uv = sampleUv;
	//vs_out.sampleIndex = sampleIndex;
}