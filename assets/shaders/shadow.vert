#version 440 core
layout(location = 0) in vec3 position;
layout(location = 5) in mat4 m;

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;

void main() {
	gl_Position = proj * view * m * vec4(position, 1);
}
