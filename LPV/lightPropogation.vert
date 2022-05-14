#version 420 core
layout(location = 0) in vec3 aGridIndex;
layout(location = 1) in vec3 aOriginIndex;
layout(location = 2) in float aSampleIndex;

out VS_OUT {
    flat ivec3 gridIndex;
	flat ivec3 originIndex;
    flat int sampleIndex;
} vs_out;

void main() {
	vs_out.gridIndex = ivec3(int(floor(aGridIndex.x + 0.5f)), int(floor(aGridIndex.y + 0.5f)), int(floor(aGridIndex.z + 0.5f)));
	vs_out.originIndex = ivec3(int(floor(aOriginIndex.x + 0.5f)), int(floor(aOriginIndex.y + 0.5f)), int(floor(aOriginIndex.z + 0.5f)));
	vs_out.sampleIndex = int(floor(aSampleIndex + 0.5f));
}