#version 440 core
layout(location = 0) in vec3 position;
layout(location = 5) in mat4 m;
layout (location = 9) in vec4 influences;
layout (location = 10) in ivec4 controllers;
layout (location = 11) uniform mat4 currentSkeletonTransformation[100]; //Maximum of 100 joints per skeleton

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;
layout(location = 2) uniform bool animation;

void main() {
	if(animation) {
		vec4 finalVertexPos = vec4(0.0);

		for (int i = 0; i < 4; i++) {
			int j = controllers[i];
			mat4 jointTrans = currentSkeletonTransformation[j];

			vec4 posePos = jointTrans * vec4(position, 1.0);
			finalVertexPos += posePos * influences[i];
		}
		gl_Position = proj * view * m * finalVertexPos;
	}
	else
		gl_Position = proj * view * m * vec4(position, 1);
}