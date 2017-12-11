#version 440 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 5) in mat4 m;

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;

out vec3 vertNormal;

void main() {
	mat3 normalMatrix = inverse(transpose(mat3(m)));
	vertNormal = (m * vec4(normal, 1.0f)).xyz;
	gl_Position = proj * view * m * vec4(position, 1);
}