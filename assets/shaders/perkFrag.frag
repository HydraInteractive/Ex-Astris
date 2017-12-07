#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} inData;

layout (early_fragment_tests) in;

layout (location = 4) uniform vec3 colour;

layout (location = 0) out vec4 fragColor;

void main() {

	fragColor = vec4(colour, 1.0f);
}