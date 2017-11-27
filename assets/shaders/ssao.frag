#version 440 core

out float fragColor;

in vec2 texCoords;

layout(location = 0) uniform sampler2D positions;
layout(location = 1) uniform sampler2D normals;
layout(location = 2) uniform sampler2D texNoise;

layout(location = 3) uniform mat4 projection;
layout(location = 4) uniform float bias;
layout(location = 5) uniform float kernelRadius;
layout(location = 6) uniform mat4 view;
layout(location = 7) uniform sampler2D depthBuffer;
layout(location = 8) uniform vec2 screenSize;
layout(location = 10) uniform vec3 samples[64];

const vec2 noiseScale = vec2(screenSize.x / 4, screenSize.y / 4);

float linearizeDepth(float near, float far, float depth) {
    return 2.0 * near / (far + near - depth * (far - near));
}

vec3 positionFromDepth(vec2 texcoords) {
    float d = linearizeDepth(0.1f, 75.0f, texture(depthBuffer, texcoords).r);
    vec4 pos = inverse(projection) * vec4(texcoords.x * 2.0 - 1.0, texcoords.y * 2.0 - 1.0, d * 2.0 - 1.0, 1.0);

    pos.xyz /= pos.w;

    return pos.xyz;
}

vec3 normal_from_depth(float depth, vec2 texcoords) {
  const vec2 offset1 = vec2(0.0, 0.001);
  const vec2 offset2 = vec2(0.001, 0.0);

  float depth1 = linearizeDepth(0.1f, 75.0f, texture(depthBuffer, texcoords + offset1).r);
  float depth2 = linearizeDepth(0.1f, 75.0f, texture(depthBuffer, texcoords + offset2).r);

  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);

  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;

  return normalize(normal);
}

void main() {
	//vec3 fragPos = texture(positions, texCoords).xyz;
	vec3 normal = normalize(texture(normals, texCoords).xyz);

	vec3 fragPos = vec3(view * texture(positions, texCoords)).xyz;
	

	//float bias = 1.0f;
	//float kernelRadius = 2.0f;
	int kernelSize = 16;
	
	vec3 randomVec = normalize(texture(texNoise, texCoords * noiseScale).xyz);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; i++) {
		vec3 samplePoint = TBN * samples[i];
		samplePoint = fragPos + samplePoint * kernelRadius;
		//samplePoint = samplePoint * kernelRadius + origin.xyz;

		vec4 offset = vec4(samplePoint, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;	
		
		float sampleDepth = (view * vec4(texture(positions, offset.xy))).z; // hejhej nils
		//float sampleDepth = positionFromDepth(offset.xy).z;
		//float sampleDepth = texture(positions, offset.xy).z;
		//occlusion +=  (sampleDepth >= samplePoint.z + bias ? 1.0 : 0.0);

		//float rangeCheck = abs(fragPos.z - sampleDepth) < kernelRadius ? 1.0 : 0.0;
		float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(fragPos.z - sampleDepth));
		//occlusion += (sampleDepth >= samplePoint.z ? 1.0 : 0.0) * rangeCheck; 	
		occlusion += (sampleDepth >= samplePoint.z + bias ? 1.0 : 0.0) * rangeCheck; 	
	}
	
	occlusion = 1.0 - (occlusion / kernelSize);
	//fragColor = pow(occlusion, 4);
	fragColor = occlusion;
}
