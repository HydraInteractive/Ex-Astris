#version 440 core

in GeometryData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat3 tbn;
} inData;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 depthColor;

void main() {
	fragColor = inData.color;
	depthColor = vec3(gl_FragCoord.z);
}
