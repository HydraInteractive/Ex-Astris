#version 440 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	vec3 tangent;
	mat4 m;
} inData[];

out GeometryData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
} outData;

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

layout(location = 0) uniform mat4 v;
layout(location = 1) uniform mat4 p;
layout(location = 2) uniform vec3 cameraPos;

#define M_PI 3.1415

void main() {
	int i;

	for (i = 0; i < 3; i++) {
		vec4 pos = inData[i].m * vec4(inData[i].position, 1.0f);
		outData.position = pos.xyz;

		mat3 normalMatrix = transpose(inverse(mat3(inData[i].m)));
		outData.normal = normalize(normalMatrix * inData[i].normal);

		outData.color = inData[i].color;
		outData.uv = vec2(inData[i].uv.x, 1 - inData[i].uv.y);

		gl_Position = p * v * pos;
		EmitVertex();
	}
	EndPrimitive();
}
