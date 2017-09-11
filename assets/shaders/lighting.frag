#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 fragOutput;
layout(location = 1) out vec3 brightOutput;
layout(location = 0) uniform sampler2D positions;
layout(location = 1) uniform sampler2D colors;
layout(location = 2) uniform sampler2D normals;
layout(location = 3) uniform vec3 cameraPos;

void main() {
	vec3 pos = texture(positions, texCoords).rgb;
	vec3 objectColor = texture(colors, texCoords).rgb;
	vec3 normal = texture(normals, texCoords).rgb;

	vec3 lightPos = cameraPos;
	vec3 lightColor = vec3(1,1,1);

	vec3 lightDir = normalize(lightPos - pos);
	float diff = max(dot(-normal, lightDir), 0.0f);
	vec3 diffuse = lightColor * diff;

	float specularStrength = 0.1f;
	vec3 viewDir = normalize(cameraPos - pos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = spec * specularStrength * lightColor;

	vec3 ambient = lightColor * 0.3f;
	// All normal lighting calculations 
	fragOutput = (ambient + diffuse + specular) * objectColor;
	
	// Picking out bright regions for glow.
	float brightness = dot(fragOutput, vec3(0.7));
	if(brightness > 1.0f)
		brightOutput = fragOutput;
	else
		brightOutput = vec3(0);
}