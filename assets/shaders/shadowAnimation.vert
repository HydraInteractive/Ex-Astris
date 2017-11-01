#version 440 core
layout (location = 0) in vec3 position;
layout (location = 5) in mat4 m;
layout (location = 9) in vec4 weight;
layout (location = 10) in ivec4 jointIdx;

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;

layout(location = 20) uniform sampler2DRect animationTexture;

mat4 getMat(sampler2DRect tex, int joint)
{
    int x = joint * 4;

    mat4 mat = mat4(texelFetch(tex, ivec2((x + 0), gl_InstanceID)),
                    texelFetch(tex, ivec2((x + 1), gl_InstanceID)),
                    texelFetch(tex, ivec2((x + 2), gl_InstanceID)),
                    texelFetch(tex, ivec2((x + 3), gl_InstanceID)));
    return mat;
}

vec4 getFinal(vec4 beginningVec)
{
    vec4 animPos = vec4(0.0);
    animPos += weight[0] * (getMat(animationTexture, jointIdx[0]) * beginningVec);
    animPos += weight[1] * (getMat(animationTexture, jointIdx[1]) * beginningVec);
    animPos += weight[2] * (getMat(animationTexture, jointIdx[2]) * beginningVec);
    animPos += weight[3] * (getMat(animationTexture, jointIdx[3]) * beginningVec);
    return animPos;
}

void main() {
	gl_Position = proj * view * m * getFinal(vec4(position, 1.0f));
}