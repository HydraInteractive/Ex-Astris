#version 440 core

layout(location = 0) uniform sampler2D ssaoTex;

in vec2 texCoords;

layout(location = 0) out float res;

void main() {
	int blurSize = 4;

	vec2 texelSize = 1.0 / vec2(textureSize(ssaoTex, 0));

	float result = 0.0;
	vec2 hlim = vec2(float(-blurSize) * 0.5 + 0.5);
	for(int i = 0; i < blurSize; i++) {
		for(int j = 0; j < blurSize; j++) {
			vec2 offset = (hlim + vec2(float(i), float(j))) * texelSize;
			result += texture(ssaoTex, texCoords + offset).r;
		}
	}
	res = result / float(blurSize * blurSize);
}