#version 440 core

out float fragColor;

in vec2 texCoords;

layout(location = 0) uniform sampler2DMS positions;
layout(location = 1) uniform sampler2DMS normals;
layout(location = 2) uniform sampler2D texNoise;

layout(location = 3) uniform mat4 projection;
//layout(location = 4) uniform mat4 view;

layout(location = 11) uniform float bias;
layout(location = 12) uniform float kernelRadius;
layout(location = 13) uniform int kernelSize;
layout(location = 14) uniform vec3 samples[64];

const vec2 noiseScale = vec2(1920.0 / 4.0, 1080.0 / 4.0);

void main() {
	ivec2 iTexCoords = ivec2(texCoords * textureSize(positions));
	vec3 fragPos = texelFetch(positions, iTexCoords, 0).xyz;
	vec3 normal = texelFetch(normals, iTexCoords, 0).rgb ;
	

	//vec3 fragPos = vec3(0);
	//vec3 normal = vec3(0);
	//for(int i = 1; i < 5; i++) {
	//	fragPos += texelFetch(positions, iTexCoords, i).xyz; 
	//	normal += texelFetch(normals, iTexCoords, i).rgb; 
	//}
	//fragPos /= 4;
	//normal /= 4;
	//vec3 fragPos = texelFetch(positions, iTexCoords, 0).xyz; 
	//vec3 normal = texelFetch(normals, iTexCoords, 0).rgb; 

	

	//fragPos = vec3(view * vec4(fragPos, 1)).xyz;
	//normal = vec3(view * vec4(normal, 1)).rgb;
	//normal = normalize(normal);
	//fragPos = normalize(fragPos);
	
	vec3 randomVec = texture(texNoise, texCoords * noiseScale).xyz;
	//randomVec = vec3(view * vec4(randomVec, 1)).xyz;
	//randomVec = normalize(randomVec);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; i++) {
		vec3 samplePoint = TBN * samples[i];
		samplePoint = fragPos + samplePoint * kernelRadius;
		//samplePoint = vec3(inverse(view) * vec4(samplePoint, 1)).xyz;

		vec4 offset = vec4(samplePoint, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;	
		
		ivec2 iOffset = ivec2(offset.xy * textureSize(positions));
		//float sampleDepth = (view * texelFetch(positions, iOffset, 0)).z;
		float sampleDepth = texelFetch(positions, iOffset, 0).z;
	



		//float sampleDepth = 0;
		//for(int j = 1; j < 5; j++) {
		//	sampleDepth += texelFetch(positions, iOffset, j).z;
		//}
		//sampleDepth /= 4;
		//sampleDepth = (view * vec4(vec3(sampleDepth), 1)).z;
		//float sampleDepth = (view * texelFetch(positions, iOffset, 0)).z;

		float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePoint.z + bias ? 1.0 : 0.0) * rangeCheck; 	
	}
	
	occlusion = 1.0 - (occlusion / kernelSize);
	fragColor = occlusion;
}