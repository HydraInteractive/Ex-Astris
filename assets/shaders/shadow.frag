#version 440 core

layout (location = 5) uniform sampler2D diffuseTexture;
layout (location = 6) uniform sampler2D normalTexture;

void main() {
	vec3 bog = texture(diffuseTexture, vec2(0)).rgb;
	bog = texture(normalTexture, vec2(0)).rgb;
	//gl_FragDepth = gl_FragCoord.z;
}