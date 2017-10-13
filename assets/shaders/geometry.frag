#version 440 core

in GeometryData {
	vec3 position;
	vec3 vPos;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat3 tbn;
	vec4 light;
} inData;

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 diffuse;
layout (location = 2) out vec3 normal;
layout (location = 3) out vec4 lightPos;
layout (location = 5) out vec3 viewPos;

void main() {
	position = inData.position;
	diffuse = inData.color;
	normal = inData.normal;
	lightPos = inData.light;
	viewPos = inData.vPos;
}