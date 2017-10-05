#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat3 tbn;
	vec4 light;
} inData;

layout (location = 0) out vec3 finalColor;

void main() {
	finalColor = vec3(1,0,0);
}


