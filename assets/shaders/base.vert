#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;

out VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} outData;

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

void main() {
	outData.position = position;
	outData.normal = normal;
	outData.color = color;
	outData.uv = uv;

	gl_Position = vec4(position, 1);
}
