#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;

out vec2 texCoords;

void main() {
	texCoords = uv;
	gl_Position = vec4(position, 1);
}
