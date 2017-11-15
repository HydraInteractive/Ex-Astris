#version 440 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 6) out;

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	mat4 m;
	vec4 charRect;
	vec3 charPos;
} inData[];

out GeometryData {
	vec3 color;
	vec2 uv;
} outData;

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

layout(location = 0) uniform mat4 vp;

void main() {
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < 3; i++) {
			outData.color = inData[i].color * (1 - j * 0.75);
			// vUV      = [0..1, 0..1]
			// vCharRect.xy = [startX, startY] in texture
			// vCharRect.zw = [width, height] of char
			// vCharPos     = [xPos, yPos]
			outData.uv = inData[i].uv * inData[i].charRect.zw + inData[i].charRect.xy;

			gl_Position = vp * inData[i].m * (vec4(inData[i].position * vec3(inData[i].charRect.zw, 1) + inData[i].charPos, 1) + vec4(j * -0.01, j * 0.005, j * 0.01, 0));
			EmitVertex();
		}
		EndPrimitive();
	}
}
