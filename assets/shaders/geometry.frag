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

layout (location = 5) uniform sampler2D diffuseTexture;
layout (location = 6) uniform sampler2D normalTexture;

void main() {
	position = inData.position;
	diffuse = texture(diffuseTexture, inData.uv).rgb;
	vec3 tempNormal = texture(normalTexture, inData.uv).rgb * 2 - 1;
	normal = normalize(inData.tbn * tempNormal);
	lightPos = inData.light;
	viewPos = inData.vPos;
}