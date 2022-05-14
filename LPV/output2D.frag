#version 420 core

in vec2 fTexCoords;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main() {
	vec3 color = texture(screenTexture, fTexCoords).xyz;
	FragColor = vec4(color, 1.f);
}