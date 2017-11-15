#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	vec3 tangent;
	mat4 m;
} inData;


layout (location = 1) uniform sampler2D diffuse;
layout (location = 2) uniform sampler2D depth;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = texture(diffuse, uv);
	gl_FragDepth = texture(depth, uv).r;
}
