#version 440 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;
layout (location = 4) in vec3 tangent;
layout (location = 5) in mat4 m;
layout (location = 9) in vec4 weight;
layout (location = 10) in ivec4 jointIdx;

//layout (location = 11) uniform mat4 currentSkeletonTransformation[100]; //Maximum of 100 joints per skeleton

layout(location = 11) uniform sampler2DRect animationTexture;


out VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	vec3 tangent;
	mat4 m;
} outData;

mat4 getMat(sampler2DRect tex, int joint) {
	int x = joint * 4;
	mat4 mat = mat4(texelFetch(tex, ivec2((x + 0), gl_InstanceID)),
	                texelFetch(tex, ivec2((x + 1), gl_InstanceID)),
	                texelFetch(tex, ivec2((x + 2), gl_InstanceID)),
	                texelFetch(tex, ivec2((x + 3), gl_InstanceID)));
    return mat;
}

mat3 getMatForNormal(sampler2DRect tex, int joint) {
    return transpose(inverse(mat3(getMat(tex, joint))));
}

vec4 getFinal(vec4 beginningVec) {
	vec4 animPos = vec4(0.0);
	animPos += weight[0] * (getMat(animationTexture, jointIdx[0]) * beginningVec);
	animPos += weight[1] * (getMat(animationTexture, jointIdx[1]) * beginningVec);
	animPos += weight[2] * (getMat(animationTexture, jointIdx[2]) * beginningVec);
	animPos += weight[3] * (getMat(animationTexture, jointIdx[3]) * beginningVec);
	return animPos;
}

vec3 getNormal(vec3 beginningVec){
	vec3 animNormal = vec3(0);
	animNormal += weight[0] * (getMatForNormal(animationTexture, jointIdx[0]) * beginningVec);
	animNormal += weight[1] * (getMatForNormal(animationTexture, jointIdx[1]) * beginningVec);
	animNormal += weight[2] * (getMatForNormal(animationTexture, jointIdx[2]) * beginningVec);
	animNormal += weight[3] * (getMatForNormal(animationTexture, jointIdx[3]) * beginningVec);
	return animNormal;
};


void main() {
	outData.position = getFinal(vec4(position, 1.0)).xyz;
	outData.normal = getNormal(normal);
	outData.color = color;
	outData.uv = uv;
	outData.tangent = tangent;
	outData.m = m;
}
