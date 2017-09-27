#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 7) uniform mat4 lightView;
layout (location = 8) uniform mat4 lightProj;


layout(location = 5) in mat4 m;

out vec2 texCoords;
out vec4 lightSpacePos;


void main() {
	texCoords = uv;
	lightSpacePos = lightProj * lightView * m * vec4(position, 1);
	gl_Position = vec4(position, 1);
}
