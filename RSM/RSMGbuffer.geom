#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 lightShadowMatrices[6];

in VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
	vec2 fTexCoords;

} gs_in[];

out VS_OUT {
	vec3 worldPos;
	vec3 fNormal;
	vec2 fTexCoords;
} gs_out;

void main() {
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            gs_out.worldPos = gs_in[i].worldPos;
            gs_out.fNormal = gs_in[i].fNormal;
            gs_out.fTexCoords = gs_in[i].fTexCoords;
            gl_Position = lightShadowMatrices[face] * gl_in[i].gl_Position;
            EmitVertex();
        }    
        EndPrimitive();
    }
}