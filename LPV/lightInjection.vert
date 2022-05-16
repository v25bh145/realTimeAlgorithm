#version 420 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

void main() {
    TexCoords = aPos;
    gl_Position = vec4(aPos.xyz, 1.f);
}