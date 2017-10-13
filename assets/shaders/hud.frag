#version 440 core

in vec2 texCoords;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform sampler2D diffuse;

void main() {
	vec4 color = texture(diffuse, texCoords);

	fragColor = color;
}