#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 5) in mat4 m;
layout (location = 14) in vec4 charRect;
layout (location = 15) in vec3 charPos;

out VertexData {
	vec3 color;
	vec2 uv;
} outData;

layout(location = 0) uniform mat4 vp;
layout(location = 2) uniform mat4 model;
layout(location = 3) uniform vec3 rightVector;
layout(location = 4) uniform vec3 upVector;

void main() {
	outData.color = vec3(1,1,1);
	vec2 flippedUV = vec2(uv.x, 1 - uv.y);
	outData.uv = flippedUV * charRect.zw + charRect.xy;
	
	// Fix billboarded effect.
	vec3 billboardPos = (vec3(-charPos.x * rightVector) + vec3(-charPos.y * upVector)) + vec3(rightVector * position.x * charRect.z) + vec3(upVector * position.y * charRect.w);

	gl_Position = vp * model * vec4(billboardPos, 1);
}
