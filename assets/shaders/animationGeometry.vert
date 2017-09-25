#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 5) in mat4 m;
layout (location = 9) in vec4 influences;
layout (location = 10) in ivec4 controllers;
layout (location = 11) uniform mat4 currentSkeletonTransformation[100]; //Maximum of 100 joints per skeleton

out VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	vec3 tangent;
	mat4 m;
} outData;

void main() {
	vec4 finalVertexPos = vec4(0.0);
	vec4 finalNormal = vec4(0.0);

	for (int i = 0; i < 4; i++) {
		int j = controllers[i];
		mat4 jointTrans = currentSkeletonTransformation[j];

		vec4 posePos = jointTrans * vec4(position, 1.0);
		finalVertexPos += posePos * influences[i];

		vec4 worldNormal = jointTrans * vec4(normal, 0.0);
		finalNormal += worldNormal * influences[i];
	}

	outData.position = finalVertexPos.xyz;
	outData.normal = finalNormal.xyz;
	outData.color = color;
	outData.uv = uv;
	outData.tangent = tangent;
	outData.m = m;
}
