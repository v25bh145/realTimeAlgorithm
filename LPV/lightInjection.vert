#version 420 core
layout(location = 0) in vec3 uv;

uniform samplerCube worldPosMap;
uniform samplerCube fluxMap;

out VS_OUT {
	vec3 worldPos;
	vec3 flux;
	vec3 uv;
} vs_out;

void main() {

	vs_out.worldPos = texture(worldPosMap, uv).xyz;
	vs_out.flux = texture(fluxMap, uv).xyz;
	vs_out.uv = uv;
}