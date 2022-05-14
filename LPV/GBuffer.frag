#version 420 core
layout (location = 0) out vec3 worldPos;
layout (location = 1) out vec3 diffuse;
layout (location = 2) out vec3 normal;

in VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
	vec2 fTexCoords;
} fs_in;

uniform sampler2D texture_diffuse1;

void main() {
	diffuse = texture(texture_diffuse1, fs_in.fTexCoords).xyz;
	normal = normalize(fs_in.fNormal);
	worldPos = fs_in.worldPos;
}