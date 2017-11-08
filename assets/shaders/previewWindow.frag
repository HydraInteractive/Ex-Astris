#version 440 core

in VertexData {
	vec3 position;
	vec3 normal;
	vec3 color;
	vec2 uv;
	vec3 tangent;
	mat4 m;
} inData;

layout (location = 0) out vec3 finalColor;
layout (location = 2) uniform vec3 cameraPos;

void main() {
	vec3 lightPos = cameraPos;
	vec3 lightColor = vec3(1,1,1);
	vec3 lightDir = lightPos - inData.position;
	vec3 normal = normalize(inData.normal);

	vec3 lightDirection = normalize(lightDir);
	float diff = max(dot(normal, lightDirection), 0.0f);
	vec3 diffuse = lightColor * diff;

	float specularStrength = 0.1f;
	vec3 viewDir = normalize(cameraPos - inData.position);
	vec3 reflectDir = reflect(-lightDirection, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = spec * specularStrength * lightColor;

	vec3 ambient = lightColor * 0.3f;
	// All normal lighting calculations 
	finalColor = (diffuse + specular + ambient) * inData.color;
}
