#version 440 core

in GeometryData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat3 tbn;
} inData;

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 diffuse;
layout (location = 2) out vec3 normal;
layout (location = 3) out vec3 depth;

void main() {
	position = inData.position;
	diffuse = inData.color;
	normal = inData.normal;
	depth = vec3((gl_FragCoord.z / gl_FragCoord.w)/ 2 + 0.5);
}
