#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 5) in mat4 m;

out VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	vec3 tangent;
	mat4 m;
} outData;

void main() {
	outData.position = position;
	outData.normal = normal;
	outData.color = color;
	outData.uv = uv;
	outData.tangent = tangent;
	outData.m = m;
}
