#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 5) in mat4 m;
layout (location = 14) in vec4 charRect;
layout (location = 15) in vec3 charPos;

out VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat4 m;
	vec4 charRect;
	vec3 charPos;
} outData;

void main() {
	outData.position = position;
	outData.normal = normal;
	outData.color = color;
	outData.uv = uv;
	outData.m = m;
	outData.charRect = charRect;
	outData.charPos = charPos;
}
