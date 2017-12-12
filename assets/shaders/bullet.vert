#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 5) in mat4 m;


layout (location = 0) uniform mat4 view;
layout (location = 1) uniform mat4 proj;

out VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} outData;

void main() {
	outData.position = (m * vec4(position, 1.0)).xyz;
	outData.normal = normal;
	outData.uv = uv;

	gl_Position = proj * view * m * vec4(position, 1);
}
