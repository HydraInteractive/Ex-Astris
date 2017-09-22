#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 fragOutput;
layout(location = 1) out vec3 brightOutput;
layout(location = 0) uniform sampler2DMS positions;
layout(location = 1) uniform sampler2DMS colors;
layout(location = 2) uniform sampler2DMS normals;
layout(location = 3) uniform sampler2DMS lightPositions;
layout(location = 4) uniform sampler2DShadow depthMap;
layout(location = 5) uniform vec3 cameraPos;
layout(location = 6) uniform vec3 lightDir;

void main() {
	ivec2 iTexCoords = ivec2(texCoords * textureSize(positions));
	vec3 pos = vec3(0);
	vec3 objectColor = vec3(0);
	vec3 normal = vec3(0);
	vec4 lightPos = vec4(0);
	for(int i = 1; i < 5; i++){
		pos += texelFetch(positions, iTexCoords, i).rgb;
		objectColor += texelFetch(colors, iTexCoords, i).rgb;
		normal += texelFetch(normals, iTexCoords, i).rgb;
		lightPos += texelFetch(lightPositions, iTexCoords, i);
	}

	pos /= 4;
	objectColor /= 4;
	normal /= 4;
	lightPos /= 4;
/*
	vec3 lightColor = objectColor;//vec3(1,1,1);

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
	float inShadow = (lightPos.w > 1) ? textureProj(depthMap, lightPos) : 1;
	fragOutput = (diffuse + specular) * inShadow + ambient;
	*/

	//fragOutput *= objectColor;

	fragOutput = vec3(0, 1, 0);//normal; //;vec3(inShadow)*0.75 + *0.25;

	// Picking out bright regions for glow.
	float brightness = dot(fragOutput, vec3(0.7));
	if(brightness > 1.0f)
		brightOutput = fragOutput;
	else
		brightOutput = vec3(0);

}