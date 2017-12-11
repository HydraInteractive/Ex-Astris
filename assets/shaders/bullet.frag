#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} inData;

layout (early_fragment_tests) in;

//layout (location = 4) uniform vec3 colour;
layout(location = 5) uniform vec3 colour;
layout(location = 6) uniform bool hasGlow;
layout(location = 7) uniform float glowIntensity;

layout (location = 1) out vec4 diffuse;
layout(location = 4) out float glow;
//layout (location = 0) out vec4 fragColor;

void main() {
	//position = inData.position;
	if(hasGlow)
	{
		glow = glowIntensity;
	}
	else
		glow = 0;

	
	diffuse = vec4(colour, 1.0f);

	//fragColor = vec4(colour, 1.0f);
}