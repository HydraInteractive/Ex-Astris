#version 440 core

//layout (early_fragment_tests) in;

layout(location = 2) uniform vec3 direction;

in vec3 vertNormal;

void main() {
	float bias = 0.005 * (1 - dot(normalize(vertNormal), -normalize(direction)));
	gl_FragDepth = gl_FragCoord.z + bias;
}