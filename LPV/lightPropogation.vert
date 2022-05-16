#version 420 core
//layout(location = 0) in float aGridIndex;

out flat int iGridIndex;

void main() {
	iGridIndex = gl_VertexID;
}