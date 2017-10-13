#version 440 core

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 uv;
layout (location = 5) in mat4 m;

out vec2 texCoords;

void main() {
	texCoords = uv;
	gl_Position =  m * vec4(position, 1);
}