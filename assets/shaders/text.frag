#version 440 core

in VertexData {
	vec3 color;
	vec2 uv;
} inData;

layout(location = 0) out vec4 fragOutput;

layout(location = 1) uniform sampler2D charTexture;
layout(location = 5) uniform float alphaFade;

void main() {
	vec4 charTex = texture(charTexture, inData.uv);
	if(charTex.a < 0.5f)
		discard;

	fragOutput = vec4(charTex.xyz * inData.color, clamp(alphaFade, 0.0, 1.0));
}