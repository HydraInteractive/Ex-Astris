#version 440 core

in vec2 texCoords;

out vec4 fragColor;

layout(location = 0) uniform sampler2D finalImage;

void main() {
	vec3 pixel = texture(finalImage, texCoords).rgb;
		
	fragColor = vec4(pixel, 1);
}
