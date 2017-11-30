#version 440 core

out float fragColor;

in vec2 texCoords;

const int kernelSize = 4;

layout(location = 0) uniform sampler2D positions;
//layout(location = 1) uniform sampler2D normals;
layout(location = 2) uniform sampler2D texNoise;

layout(location = 3) uniform mat4 projection;
//layout(location = 4) uniform float bias;
//layout(location = 5) uniform float kernelRadius;
layout(location = 6) uniform mat4 view;
layout(location = 7) uniform sampler2D depthBuffer;
layout(location = 8) uniform vec2 screenSize;
layout(location = 11) uniform vec3 samples[kernelSize];

const vec2 noiseScale = vec2(screenSize.x / 4, screenSize.y / 4);

vec3 calcViewPos(vec2 uv, float depth) {
	mat4 invProj = inverse(projection);
	depth = depth * 2.0 - 1.0;
	vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, depth, 1.0);
	vec4 viewPos = invProj * clipSpacePos;
	viewPos = vec4(viewPos.xyz / viewPos.w, 1.0);

	return viewPos.xyz;
}

void main() {
	//vec3 fragPos = vec3(view * texture(positions, texCoords)).xyz;
	//vec3 normal = normalize(texture(normals, texCoords).xyz);
	vec3 fragPos = calcViewPos(texCoords, texture(depthBuffer, texCoords).r); // reconstructed from depth
	vec3 normal = normalize(cross(dFdx(fragPos), dFdy(fragPos))); // reconstruct view nornal

	

	float bias = 0.025f;
	float kernelRadius = 0.5f;
	
	
	vec3 randomVec = normalize(texture(texNoise, texCoords * noiseScale).xyz);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; i++) {
		vec3 samplePoint = TBN * samples[i];
		samplePoint = fragPos + samplePoint * kernelRadius;

		vec4 offset = vec4(samplePoint, 1.0);
		offset = projection * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;	
		
		//float sampleDepth = (view * vec4(texture(positions, offset.xy))).z; 
		float sampleDepth = vec3(calcViewPos(offset.xy, texture(depthBuffer, offset.xy).r)).z; // reconstructed from depth

		float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePoint.z + bias ? 1.0 : 0.0) * rangeCheck; 	
	}
	
	occlusion = 1.0 - (occlusion / kernelSize);
	fragColor = pow(occlusion, 4);
	//fragColor = occlusion;
}