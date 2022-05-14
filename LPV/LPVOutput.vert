#version 420 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexcoords;

out vec2 fTexCoords;

void main() {
	gl_Position = vec4(aPos, 0.f, 1.f);
	fTexCoords = aTexcoords;
}