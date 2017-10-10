#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 blurOutput;
layout(location = 1) uniform sampler2D prevGlowImage;
layout(location = 2) uniform bool horizontal;
layout(location = 3) uniform int kernelSize;
layout(location = 4) uniform float weight[5];

void main() {
	vec2 tex_offset = 1.0 / textureSize(prevGlowImage, 0); // gets size of single texel
    vec3 result = texture(prevGlowImage, texCoords).rgb * weight[0]; // current fragment's contribution
    if (horizontal) {
        for(int i = 1; i < kernelSize; ++i)
        {
            result += texture(prevGlowImage, texCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(prevGlowImage, texCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    } else {
        for(int i = 1; i < kernelSize; ++i)
        {
            result += texture(prevGlowImage, texCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(prevGlowImage, texCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
	blurOutput = result;
}
