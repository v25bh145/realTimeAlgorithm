#version 420 core

in vec3 TexCoords;

uniform samplerCube skyboxTexture;

out vec4 FragColor;

void main() {    
    vec3 color = texture(skyboxTexture, TexCoords).rgb;
    FragColor = vec4(color, 1.f);
}