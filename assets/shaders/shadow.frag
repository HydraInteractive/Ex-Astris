#version 440 core

layout (location = 20) uniform sampler2D diffuseTexture;
layout (location = 21) uniform sampler2D normalTexture;
layout (location = 22) uniform sampler2D specularTexture;
layout (location = 23) uniform sampler2D glowTexture;

void main() {
	vec3 bog = texture(diffuseTexture, vec2(0)).rgb;
	bog = texture(normalTexture, vec2(0)).rgb;
	bog = texture(specularTexture, vec2(0)).rgb;
	bog = texture(glowTexture, vec2(0)).rrr;
	//gl_FragDepth = gl_FragCoord.z;
}