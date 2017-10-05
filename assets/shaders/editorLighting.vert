#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 5) in mat4 m;

out vec2 texCoords;

void main() {
	texCoords = uv;
	texCoords.y = 1 - uv.y;
	gl_Position = vec4(position, 1);
}
