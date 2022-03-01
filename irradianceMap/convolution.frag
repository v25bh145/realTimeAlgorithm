#version 330 core

out vec4 FragColor;

in vec3 worldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main() {
	// from (0, 0, 0) to p is a normal of the unit sphere
	vec3 normal = normalize(worldPos);

	vec3 irradiance = vec3(0.f);

	// convolution
	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = cross(up, normal);
	up         = cross(normal, right);
	float sampleDelta = 0.025f;
	float nrSamples = 0.f;
	for(float phi = 0.f; phi < 2.f * PI; phi += sampleDelta)
		for(float theta = 0.f; theta < PI * 0.5f; theta += sampleDelta) {
			float sinTheta = sin(theta);
			vec3 tangentSample = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cos(theta));
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
			irradiance  += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	irradiance = PI * irradiance * (1.f / float(nrSamples));

	FragColor = vec4(irradiance, 1.f);
}