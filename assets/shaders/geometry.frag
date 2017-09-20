#version 440 core

in GeometryData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat3 tbn;
	vec3 light;
} inData;

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 diffuse;
layout (location = 2) out vec3 normal;
layout (location = 3) out vec3 depth;
layout (location = 4) out vec3 lightDepth;

void main() {
	position = inData.position;
	diffuse = inData.color;
	normal = inData.normal;
	lightDepth = inData.light;
	depth = vec3(gl_FragCoord.z / gl_FragCoord.w);
}
