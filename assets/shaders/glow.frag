#version 440 core

in vec2 texCoords;
layout(location = 0) out vec3 finalOutput;
layout(location = 1) uniform sampler2D originalImage;
layout(location = 2) uniform sampler2D blurrImage1;
layout(location = 3) uniform sampler2D blurrImage2;
layout(location = 4) uniform sampler2D blurrImage3;

void main() {
	vec3 result = texture(originalImage, texCoords).rgb;
	result += texture(blurrImage1, texCoords).rgb;
	result += texture(blurrImage2, texCoords).rgb;
	result += texture(blurrImage3, texCoords).rgb;
	finalOutput = clamp(result, 0.0, 1.0);
}
