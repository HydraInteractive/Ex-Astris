#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat3 tbn;
} inData;

layout(location = 0) uniform sampler2D prevGlowImage;
layout(location = 1) uniform bool horizontal;
layout(location = 2) out vec4 glowOutput;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216); // Precalculated from a 5x5 kernel.

void main() {
	vec2 tex_offset = 1.0 / textureSize(prevGlowImage, 0); // gets size of single texel
    vec3 result = texture(prevGlowImage, inData.uv).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(prevGlowImage, inData.uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(prevGlowImage, inData.uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(prevGlowImage, inData.uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(prevGlowImage, inData.uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    glowOutput = vec4(result, 1.0);
}
