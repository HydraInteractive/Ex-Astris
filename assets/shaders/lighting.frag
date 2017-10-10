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
layout(location = 0) uniform sampler2DMS positions;
layout(location = 1) uniform sampler2DMS colors;
layout(location = 2) uniform sampler2DMS normals;
layout(location = 3) uniform sampler2DMS lightPositions;
layout(location = 4) uniform sampler2D depthMap;
layout(location = 5) uniform vec3 cameraPos;
layout(location = 6) uniform int nrOfPointLights;
layout(location = 7) uniform DirLight dirLight;
layout(location = 9) uniform PointLight pointLights[MAX_LIGHTS];

vec3 calcPointLight(PointLight light, vec3 pos, vec3 normal, vec3 objectColor){
	float distance = length(light.pos - pos);
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	// Diffuse
	vec3 lightDir = normalize(light.pos - pos);
	float diff = max(dot(-normal, lightDir), 0.0);
	vec3 diffuse = light.color * diff * objectColor;

	// Specular
	float specularStrength = 0.1f;
	vec3 viewDir = normalize(cameraPos - pos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = spec * specularStrength * light.color;

	vec3 ambient = light.color * 1 / nrOfPointLights;
	return (diffuse + specular) * attenuation;
}

vec3 calcDirLight(DirLight light, vec3 pos, vec3 normal, vec3 objectColor){
	vec3 lightDir = normalize(-light.dir);
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = light.color * diff * objectColor;

	float specularStrength = 0.1f;
	vec3 viewDir = normalize(cameraPos - pos);
	vec3 reflectDir = reflect(lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = spec * specularStrength * light.color;

	return (diffuse + specular);
}

void main() {
	ivec2 iTexCoords1 = ivec2(texCoords * textureSize(positions));
	vec3 pos = vec3(0);
	vec3 objectColor = vec3(0);
	vec3 normal = vec3(0);
	vec4 lightPos = vec4(0);
	for(int i = 0; i < 4; i++){
		pos += texelFetch(positions, iTexCoords1, i).xyz;
		objectColor += texelFetch(colors, iTexCoords1, i).rgb;
		normal += texelFetch(normals, iTexCoords1, i).xyz;
		lightPos += texelFetch(lightPositions, iTexCoords1, i);
	}

	pos /= 4;
	objectColor /= 4;
	normal /= 4;
	lightPos /= 4;

	// Lighting 
	vec3 globalAmbient = dirLight.color * 0.3f;
	vec3 result = vec3(0);

	// Directional light
	result = calcDirLight(dirLight, pos, normal, objectColor);

	// Point Lights.s
	for(int i = 0 ; i < nrOfPointLights; i++){
		result += calcPointLight(pointLights[i], pos, normal, objectColor);
	}

	// Shadow
	vec3 projCoords = lightPos.xyz / lightPos.w;
	float closestDepth = texture(depthMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, dirLight.dir)), 0.005);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthMap, 0);

	// PCF
	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1 : 0;
		}
	}

	shadow /= 9.0;
	shadow = 1 - shadow;
	result *= shadow;

	fragOutput = result;

	// Picking out bright regions for glow.
	float brightness = dot(vec3(fragOutput), vec3(0.2126, 0.7152, 0.0722));

	if(brightness > 1.0f)
		brightOutput = vec3(fragOutput.x, 0,0);
	else
		brightOutput = vec3(0);
}
