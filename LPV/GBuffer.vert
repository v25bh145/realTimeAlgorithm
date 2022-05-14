#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
	vec2 fTexCoords;
} vs_out;

void main() {
	vs_out.worldPos = (model * vec4(aPos, 1.f)).xyz;
	vs_out.fNormal = mat3(transpose(inverse(model))) * aNormal;
	vs_out.fTexCoords = aTexCoords;
	gl_Position = projection * view * model * vec4(aPos, 1.f);
}