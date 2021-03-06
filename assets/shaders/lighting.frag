#version 440 core

in vec2 texCoords;

const int MAX_LIGHTS = 24;

struct DirLight{
	vec3 dir;
	vec3 color;
};

struct PointLight{
	vec3 pos;
	vec3 color;
	float constant;
	float linear;
	float quadratic;
};

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

layout(location = 0) out vec3 fragOutput;
layout(location = 1) out vec3 brightOutput;

layout(location = 0) uniform sampler2D positions;
layout(location = 1) uniform sampler2D colors;
layout(location = 2) uniform sampler2D normals;
layout(location = 4) uniform sampler2D depthMap;
layout(location = 5) uniform sampler2D ssao;
layout(location = 6) uniform sampler2D glow;
layout(location = 7) uniform vec3 cameraPos;
layout(location = 8) uniform bool enableSSAO = true;
layout(location = 9) uniform int nrOfPointLights;
layout(location = 10) uniform DirLight dirLight;
layout(location = 12) uniform mat4 projection;
layout(location = 13) uniform mat4 lightS;
layout(location = 14) uniform bool enableShadows = true;
layout(location = 15) uniform PointLight pointLights[MAX_LIGHTS];


float random(vec4 seed){
	float value = dot(seed, vec4(12.9898, 78.233, 45.164, 94.673));
	return fract(sin(value)) * 43758.5453;
}

vec3 calcPointLight(PointLight light, vec3 pos, vec3 normal, vec4 objectColor){
	float distance = length(light.pos - pos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// Diffuse
	vec3 lightDir = normalize(light.pos - pos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.color * diff * objectColor.rgb;

	// Specular
	vec3 viewDir = normalize(cameraPos - pos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = spec * objectColor.a * light.color;

	float fadeout = min(1, max(0, 2*2*2 - pow(length(light.pos - cameraPos) / 34, 2) + 1));

	return max(vec3(0, 0, 0), (specular + diffuse) * attenuation) * fadeout;
}

vec3 calcDirLight(DirLight light, vec3 pos, vec3 normal, vec4 objectColor){
	vec3 lightDir = normalize(-light.dir);
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = light.color * diff * objectColor.rgb;

	vec3 viewDir = normalize(cameraPos - pos);
	vec3 reflectDir = reflect(lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = spec * objectColor.a * light.color;

	return (diffuse + specular);
}

vec3 calcViewPos(vec2 uv, float depth) {
	mat4 invProj = inverse(projection);
	depth = depth * 2.0 - 1.0;
	vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, depth, 1.0);
	vec4 viewPos = invProj * clipSpacePos;
	viewPos = vec4(viewPos.xyz / viewPos.w, 1.0);

	return viewPos.xyz;
}

void main() {
	vec3 pos = texture(positions, texCoords).xyz;
	vec4 objectColor = texture(colors, texCoords);
	vec3 normal = normalize(texture(normals, texCoords).xyz);
	vec4 lightPos = lightS * vec4(pos, 1.0f);
	float glowAmnt = texture(glow, texCoords).r;

	// Lighting
	// 0.1f should be ambient coefficient
	vec3 globalAmbient = objectColor.rgb * 0.1f;
	vec3 result = vec3(0);

	// Directional light
	//result = calcDirLight(dirLight, pos, normal, objectColor);

	// Point Lights
	for(int i = 0 ; i < nrOfPointLights; i++){
		result += calcPointLight(pointLights[i], pos, normal, objectColor);
	}

	// Shadow
	vec3 projCoords = lightPos.xyz / lightPos.w;
	float currentDepth = projCoords.z;
	float shadow = 0.0f;

	// PCF - 16 Samples
	if (enableShadows) {
		vec2 texelSize = 1.0 / textureSize(depthMap, 0);
		for(float y = -1; y <= 1; y++) {
			for(float x = -1; x <= 1; x++) {
				float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
				shadow += currentDepth > pcfDepth ? 1 : 0;
			}
		}
		shadow /= 9;
	}
	shadow = 1.0f - shadow;

	if(glowAmnt > 0)
		brightOutput = objectColor.rgb * glowAmnt;
	else
		brightOutput = vec3(0);

	float ambientOcclusion = texture(ssao, texCoords).r;
	if (enableSSAO) {
		globalAmbient *= ambientOcclusion;
		result *= ambientOcclusion;
	}

	result *= shadow;

	result += globalAmbient;

	fragOutput = result;
}
