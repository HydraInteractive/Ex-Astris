#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 5) in mat4 m;
layout (location = 11) in vec2 textureOffset1;
layout (location = 12) in vec2 textureOffset2;
layout (location = 13) in vec2 textureCoordInfo;

out vec2 texCoords;
out vec2 textureCoords1;
out vec2 textureCoords2;
out float blend;

void main() {
	texCoords = uv.xy;
	texCoords.y = 1.0 - uv.y;
	texCoords /= textureCoordInfo.x;
	textureCoords1 = texCoords + textureOffset1;
	textureCoords2 = texCoords + textureOffset2;
	blend = textureCoordInfo.y;

	gl_Position = m * vec4(position, 1);
}