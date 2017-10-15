#version 440 core

layout (location = 20) uniform sampler2D diffuseTexture;
layout (location = 21) uniform sampler2D normalTexture;
layout (location = 22) uniform sampler2D specularTexture;
layout (location = 23) uniform sampler2D glowTexture;

void main() {

	//gl_FragDepth = gl_FragCoord.z;
}