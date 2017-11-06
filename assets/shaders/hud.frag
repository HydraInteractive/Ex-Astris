#version 440 core

in vec2 texCoords;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform sampler2D hudTexture;

void main() {
	fragColor = texture(hudTexture, texCoords);
}