#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	vec3 tangent;
	mat4 m;
} inData;

layout (location = 0) uniform sampler2DMS image;
layout (location = 1) uniform int samples;

layout (location = 0) out vec4 fragColor;

void main() {
	vec3 c = vec3(0, 0, 0);
	ivec2 txSize = ivec2(textureSize(image) * inData.uv);
	for(int i = 0; i < samples; i++)
		c += texelFetch(image, txSize, i).rgb;
	fragColor = vec4(1-(c / float(samples)), 1);
}
