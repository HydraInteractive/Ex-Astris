#version 440 core

in vec2 texCoords;
in vec3 posColor;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(posColor.x * 0.25, posColor.y * 0.5f, posColor.z, 1);
}