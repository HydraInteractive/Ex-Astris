#version 440 core

in GeometryData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat3 tbn;
} inData;

layout (location = 0) out vec3 position;
layout (location = 1) out vec3 diffuse;
layout (location = 2) out vec3 normal;
layout (location = 3) out vec3 depth;
layout (location = 4) out vec3 glow;

void main() {
	position = inData.position;
	diffuse = inData.color;
	normal = inData.normal;
	glow = diffuse * 0.9f;
	//float brightness = dot(vec3(diffuse), vec3(0.2126f, 0.7152f, 0.0722f));
    //if(brightness > 0.9f)
    //    glow = diffuse;

	depth = vec3(gl_FragCoord.z / gl_FragCoord.w);
}
