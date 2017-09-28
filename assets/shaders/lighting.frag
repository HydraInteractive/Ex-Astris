#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 fragOutput;
layout(location = 1) out vec3 brightOutput;
layout(location = 0) uniform sampler2DMS positions;
layout(location = 1) uniform sampler2DMS colors;
layout(location = 2) uniform sampler2DMS normals;
layout(location = 3) uniform sampler2DMS lightPositions;
layout(location = 4) uniform sampler2D depthMap;
layout(location = 5) uniform vec3 cameraPos;
layout(location = 6) uniform vec3 lightDir;

void main() {
	ivec2 iTexCoords = ivec2(texCoords * textureSize(positions));
	vec3 pos = vec3(0);
	vec3 objectColor = vec3(0);
	vec3 normal = vec3(0);
	vec4 lightPos = vec4(0);
	for(int i = 0; i < 4; i++){
		pos += texelFetch(positions, iTexCoords, i).rgb;
		objectColor += texelFetch(colors, iTexCoords, i).rgb;
		normal += texelFetch(normals, iTexCoords, i).rgb;
		lightPos += texelFetch(lightPositions, iTexCoords, i);
	}

	pos /= 4;
	objectColor /= 4;
	normal /= 4;
	lightPos /= 4;
	//lightPos = texelFetch(lightPositions, iTexCoords, 0);

	vec3 lightColor = normal;//vec3(1,1,1);

	vec3 lightDirection = normalize(lightDir);
	float diff = max(dot(normal, lightDirection), 0.0f);
	vec3 diffuse = lightColor * diff;

	float specularStrength = 0.1f;
	vec3 viewDir = normalize(cameraPos - pos);
	vec3 reflectDir = reflect(-lightDirection, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = spec * specularStrength * lightColor;

	vec3 ambient = lightColor * 0.3f;
	// All normal lighting calculations 
	//fragOutput = diffuse + specular + ambient;

	
	

	//float shadow = 1;
	//if (lightPos.w > 1)  {
	//	shadow += textureProj(depthMap, lightPos);
	//}
	
	//vec3 projCoords = lightPos.xyz / lightPos.w;
	//projCoords = projCoords * 0.5 + 0.5;
	
	float closestDepth = texture(depthMap, lightPos.xy).r;
	float currentDepth = lightPos.z;
	
	//float shadow = currentDepth >= closestDepth ? 1.0 : 0.0;
	
	//fragOutput = normal;
	fragOutput = (diffuse + specular + ambient) * objectColor;
	if(currentDepth > closestDepth + 0.001)
		fragOutput = vec3(0, 0, 0);


	//fragOutput = vec3(currentDepth);
	//fragOutput = normal * shadow;
	//fragOutput = lightPos.xyz;

	// Picking out bright regions for glow.
	float brightness = dot(fragOutput, vec3(0.2));
	if(brightness > 1.0f)
		brightOutput = fragOutput;
	else
		brightOutput = vec3(0);

}