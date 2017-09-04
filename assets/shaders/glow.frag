#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 blurOutput;
layout(location = 1) uniform sampler2D prevGlowImage;
layout(location = 2) uniform bool firstPass;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216); // Precalculated from a 5x5 kernel.

void main() {
	vec2 tex_offset = 1.0 / textureSize(prevGlowImage, 0); // gets size of single texel
    vec3 result = texture(prevGlowImage, texCoords).rgb * weight[0]; // current fragment's contribution
    if (firstPass) {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(prevGlowImage, texCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(prevGlowImage, texCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    } else {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(prevGlowImage, texCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(prevGlowImage, texCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
	blurOutput = result;
	//finalOutput = vec3(0,1,0);
}
