#version 440 core

in vec2 texCoords;
in vec2 textureCoords1;
in vec2 textureCoords2;
in float blend;

layout (location = 0) out vec4 fragColor;
layout (location = 0) uniform sampler2D atlas;

void main() {
	vec4 color1 = texture(atlas, textureCoords1);
	vec4 color2 = texture(atlas, textureCoords2);

	//fragColor = vec4(mix(color1.xyz, color2.xyz, blend), 1);
	fragColor = vec4(1,0,0,1);
}