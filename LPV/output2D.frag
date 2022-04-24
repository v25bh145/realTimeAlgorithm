#version 420 core

in vec2 fTexCoords;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main() {
	vec4 color = texture(screenTexture, fTexCoords);
	FragColor = vec4(color.x == 0.f ? 0.5f : color.x);
	FragColor = vec4(0.5f, 0.5f, 1.f, 1.f);
}