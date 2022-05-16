#version 420 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 lightShadowMatrices[6];

in vec3 TexCoords[];

out vec3 fTexCoords;

void main() {
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            fTexCoords = TexCoords[i];
            gl_Position = lightShadowMatrices[face] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}