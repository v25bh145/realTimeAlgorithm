#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;

out VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
} vs_out;

void main() {
	vs_out.worldPos = (model * vec4(aPos, 1.f)).xyz;
	vs_out.fNormal = mat3(transpose(inverse(model))) * aNormal;
	gl_Position = model * vec4(aPos, 1.f);
}