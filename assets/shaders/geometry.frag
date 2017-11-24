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

layout (early_fragment_tests) in;

layout (location = 0) out vec3 position;
layout (location = 1) out vec4 diffuse;
layout (location = 2) out vec3 normal;
layout (location = 3) out vec4 lightPos;
layout (location = 4) out vec3 viewPos;
layout (location = 5) out float glow;

layout (location = 20) uniform sampler2D diffuseTexture;
layout (location = 21) uniform sampler2D normalTexture;
layout (location = 22) uniform sampler2D specularTexture;
layout (location = 23) uniform sampler2D glowTexture;

void main() {
	vec3 materialDiffuse = texture(diffuseTexture, inData.uv).rgb;
	float specular = texture(specularTexture, inData.uv).r;
	diffuse = vec4(materialDiffuse, specular);

	vec3 tempNormal = texture(normalTexture, inData.uv).rgb * 2 - 1;
	//normal = normalize(inData.tbn * tempNormal);
	normal = inData.normal;
	glow = texture(glowTexture, inData.uv).r;

	position = inData.position;
	lightPos = inData.light;
	viewPos = inData.vPos;
}