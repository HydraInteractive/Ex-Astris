#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} inData;

out vec4 fragColor;

void main() {
	fragColor = vec4(inData.color, 1);
}
