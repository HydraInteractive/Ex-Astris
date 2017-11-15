#version 440 core

in GeometryData {
	vec3 color;
	vec2 uv;
} inData;

out vec4 outColor;

layout(location = 20) uniform sampler2D charTexture;

void main() {
	vec4 charTex = texture(charTexture, inData.uv);
	//if(charTex.a < 0.5f)
	//	discard;

	outColor = vec4(charTex.xyz * inData.color, 1);
}