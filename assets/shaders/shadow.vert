#version 440 core

layout(location = 0) in vec3 position;

layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 proj;
layout(location = 5) in mat4 m;

void main() {
	gl_Position = proj * view * m * vec4(position, 1);
}