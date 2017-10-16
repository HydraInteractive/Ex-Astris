#version 440 core

out float fragColor;

in vec2 texCoords;

layout(location = 0) uniform sampler2D positions;
layout(location = 1) uniform sampler2D normals;
layout(location = 2) uniform sampler2D texNoise;

layout(location = 3) uniform mat4 projection;
layout(location = 4) uniform vec3 samples[64];

const vec2 noiseScale = vec2(1920.0 / 4.0, 1080.0 / 4.0);

void main() {
	vec3 fragPos = texture(positions, texCoords).xyz;
	vec3 normal = texture(normals, texCoords).xyz;

	float bias = 0.025f;
	float kernelRadius = 0.5f;
	float kernelSize = 8;
	
	vec3 randomVec = texture(texNoise, texCoords * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; i++) {
		vec3 samplePoint = TBN * samples[i];
		samplePoint = fragPos + samplePoint * kernelRadius;

		vec4 offset = vec4(samplePoint, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;	
		
		float sampleDepth = texture(positions, offset.xy).z;

		float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePoint.z + bias ? 1.0 : 0.0) * rangeCheck; 	
	}
	
	occlusion = 1.0 - (occlusion / kernelSize);
	fragColor = occlusion;
}