#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 finalOutput;

layout(location = 1) uniform sampler2D originalImage;
layout(location = 2) uniform sampler2D blurrImage1;
layout(location = 3) uniform bool glowEnabled;
layout(location = 4) uniform sampler2D depth;

void main() {
	float gamma = 1.0f;
	vec3 hdrColor = texture(originalImage, texCoords).rgb;
	vec3 bloomColor1 = texture(blurrImage1, texCoords).rgb;

	hdrColor += bloomColor1;

	//gl_FragDepth = texture(depth, texCoords).r;
	finalOutput = hdrColor;
}
