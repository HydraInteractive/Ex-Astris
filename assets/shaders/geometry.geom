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
	vec3 vPos;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat3 tbn;
	vec4 light;
} outData;

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

layout(location = 0) uniform mat4 v;
layout(location = 1) uniform mat4 p;
layout(location = 2) uniform vec3 cameraPos;

layout(location = 3) uniform bool setting_doBackFaceCulling = true;
layout(location = 4) uniform mat4 lightS;


#define M_PI 3.1415

mat3 calcTBN(mat3 normalMatrix, vec3 normal, int idx) {
	vec3 T = normalize(normalMatrix * inData[idx].tangent);
	vec3 N = normalize(normalMatrix * normal);
	// Gram-Schmidt process
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(T, N);
	return mat3(T, B, N);
}

void main() {
	int i;

	if (setting_doBackFaceCulling) {
		vec3 p0 = (inData[0].m * vec4(inData[0].position, 1.0f)).xyz;
		vec3 p1 = (inData[1].m * vec4(inData[1].position, 1.0f)).xyz;
		vec3 p2 = (inData[2].m * vec4(inData[2].position, 1.0f)).xyz;

		vec3 edge0 = p1 - p0;
		vec3 edge1 = p2 - p0;

		vec3 triangleNormal = cross(edge0, edge1);

		
		if (dot(normalize(p0 - cameraPos), triangleNormal) >= 0)
			return;

		//outData.normal = normalize(triangleNormal);
	}

	for (i = 0; i < 3; i++) {
		vec4 pos = inData[i].m * vec4(inData[i].position, 1.0f);
		outData.position = pos.xyz;
		outData.vPos = vec3(v * pos).xyz;

		mat3 normalMatrix = transpose(inverse(mat3(inData[i].m)));
		outData.normal = normalize(normalMatrix * inData[i].normal);

		outData.color = inData[i].color;
		outData.uv = inData[i].uv;
		outData.tbn = calcTBN(normalMatrix, inData[i].normal, i);

		gl_Position = p * v * pos;
		outData.light = lightS * pos;
		EmitVertex();
	}
	EndPrimitive();
}
