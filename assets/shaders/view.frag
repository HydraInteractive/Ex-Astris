#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	vec3 tangent;
	mat4 m;
} inData;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(1 - inData.color, 1);
}
