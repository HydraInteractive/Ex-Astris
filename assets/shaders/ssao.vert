#version 440 core

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 uv;

out vec2 texCoords;

void main() {
	texCoords = uv;
	gl_Position = vec4(position, 1);
}
