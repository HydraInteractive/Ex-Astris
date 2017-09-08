#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 finalOutput;

layout(location = 1) uniform sampler2D originalImage;
layout(location = 2) uniform sampler2D blurrImage1;
layout(location = 3) uniform sampler2D blurrImage2;
layout(location = 4) uniform sampler2D blurrImage3;

void main() {
	float gamma = 1.2f;
	vec3 hdrColor = texture(originalImage, texCoords).rgb;
	vec3 bloomColor1 = texture(blurrImage1, texCoords).rgb;
	vec3 bloomColor2 = texture(blurrImage2, texCoords).rgb;
	vec3 bloomColor3 = texture(blurrImage3, texCoords).rgb;

	hdrColor += bloomColor1;
	hdrColor += bloomColor2;
	hdrColor += bloomColor3;

	vec3 result = vec3(1.0) - exp(-hdrColor * 1.0);
	result = pow(result, vec3(1.0 / gamma));
	finalOutput = result;
}
