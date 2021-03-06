#version 440 core

in GeometryData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} inData;

layout (early_fragment_tests) in;

layout (location = 0) out vec3 position;
layout (location = 1) out vec4 diffuse;
layout (location = 2) out vec3 normal;
layout (location = 3) out float glow;

layout (location = 20) uniform sampler2D diffuseTexture;
layout (location = 21) uniform sampler2D normalTexture;
layout (location = 22) uniform sampler2D specularTexture;
layout (location = 23) uniform sampler2D glowTexture;

void main() {
	vec3 materialDiffuse = texture(diffuseTexture, inData.uv).rgb;
	float specular = texture(specularTexture, inData.uv).r;
	diffuse = vec4(materialDiffuse, specular);

	normal = inData.normal;

	glow = texture(glowTexture, inData.uv).r;

	position = inData.position;

	//gl_FragDepth = inData.vPos.z / 75.0;
}
