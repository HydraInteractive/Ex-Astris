#version 440 core

in vec2 texCoords;

const int MAX_LIGHTS = 12;

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

layout(location = 0) out vec3 fragOutput;
layout(location = 1) out vec3 brightOutput;

layout(location = 0) uniform sampler2D positions;
layout(location = 1) uniform sampler2D colors;
layout(location = 2) uniform sampler2D normals;
layout(location = 3) uniform sampler2D lightPositions;
layout(location = 4) uniform sampler2D depthMap;
layout(location = 5) uniform sampler2D ssao;
layout(location = 6) uniform sampler2D glow;
layout(location = 7) uniform vec3 cameraPos;
layout(location = 8) uniform bool enableSSAO = true;
layout(location = 9) uniform int nrOfPointLights;
layout(location = 10) uniform DirLight dirLight;
layout(location = 12) uniform PointLight pointLights[MAX_LIGHTS];

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

	return (specular + diffuse) * attenuation;
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

void main() {
	// MSAA
	//ivec2 iTexCoords = ivec2(texCoords * textureSize(positions));
	//ivec2 iTexCoords2 = ivec2(texCoords * textureSize(normals));
	//vec3 pos = vec3(0);
	//vec3 objectColor = vec3(0);
	//vec3 normal = vec3(0);
	//vec4 lightPos = vec4(0);
	//for(int i = 0; i < 4; i++){
	//	pos += texelFetch(positions, iTexCoords, i).xyz;
	//	objectColor += texelFetch(colors, iTexCoords, i).rgb;
	//	normal += texelFetch(normals, iTexCoords, i).xyz;
	//	lightPos += texelFetch(lightPositions, iTexCoords, i);
	//}
	//pos /= 4;
	//objectColor /= 4;
	//normal /= 4;
	//lightPos /= 4;

	vec3 pos = texture(positions, texCoords).xyz;
	vec4 objectColor = texture(colors, texCoords);
	vec3 normal = normalize(texture(normals, texCoords).xyz);
	vec4 lightPos = texture(lightPositions, texCoords);
	float glowAmnt = texture(glow, texCoords).r;

	// Lighting 
	// 0.1f should be ambient coefficient
	vec3 globalAmbient = dirLight.color * objectColor.rgb * 0.1f;
	vec3 result = vec3(0);

	// Directional light
	result = calcDirLight(dirLight, pos, normal, objectColor);
	
	// Point Lights
	for(int i = 0 ; i < nrOfPointLights; i++){
		result += calcPointLight(pointLights[i], pos, normal, objectColor);
	}

	float ambientOcclusion = texture(ssao, texCoords).r;
	if (enableSSAO)
		globalAmbient *= ambientOcclusion;

	// Shadow
	
	//if(lightPos.w > 1){
	//	vec3 projCoords = lightPos.xyz / lightPos.w;
	//	float closestDepth = texture(depthMap, projCoords.xy).r;
	//	float currentDepth = projCoords.z;
	//	float bias = max(0.05 * (1.0 - dot(normal, -dirLight.dir)), 0.005);
	//	float shadow = 0.0f;
	//	vec2 texelSize = 1.0 / textureSize(depthMap, 0);
	//	for(int x = -1; x <= 1; x++) {
	//		for(int y = -1; y <= 1; y++) {
	//			float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
	//			shadow += currentDepth - bias > pcfDepth ? 1 : 0;
	//		}
	//	}
	//	shadow /= 9;
	//	shadow = 1 - shadow;
	//	result *= shadow;
	//}

	result += globalAmbient;

	fragOutput = result;

	if(glowAmnt > 0)
		brightOutput = fragOutput;
	else
		brightOutput = vec3(0);
}
