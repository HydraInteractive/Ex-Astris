#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 5) in mat4 m;

layout (location = 0) uniform mat4 view;
layout (location = 1) uniform mat4 proj;

out vec3 lineColor;

void main() {
	lineColor = vec3(1, 1, 0);
	gl_Position = proj * view * m * vec4(position, 1);
}